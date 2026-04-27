#include "GameState.hpp"
#include "Lobby.hpp"
#include "Player.hpp"
#include "critical.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"
#include "play.hpp"
#include "settings.hpp"

#include <array>
#include <mutex>
#include <print>
#include <queue>
#include <thread>
#include <tuple>
#include <unistd.h>

void initializeFreeIDs(std::queue<std::uint8_t>& freeIDsQueue, std::size_t IDCount)
{
    for (std::size_t i = 0; i < IDCount; i++)
    {
        freeIDsQueue.push(i);
    }
}

void manageClient(int client_fd, std::array<Player, arraySize>& players, std::array<GameState, arraySize>& gamestates, std::array<Lobby, arraySize>& lobbies, std::queue<std::uint8_t>& freeIDs, std::mutex& dataMutex, std::mutex& disconnectMutex, std::array<std::mutex, arraySize>& gameMutexes)
{
    bool client_disconnected = false;
    bool message_sent_success;

    auto [client_id, noneAvailable] = critical::getAvailableID(freeIDs, dataMutex);
    if (noneAvailable)
    {
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
        return;
    }
    auto [client_player, disconnectedTmp0, isHosting] = matchmaking::getClientInfo(client_fd, client_id);
    client_disconnected = disconnectedTmp0;

    if (client_disconnected)
    {
        critical::invalidatePlayer(players, client_id, dataMutex);
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        networking::closeFd(client_fd);
        return;
    }
    const bool playerAdded = critical::addPlayerToPlayers(players, client_player, dataMutex);
    if (!playerAdded)
    {
        std::print(stderr, "Error: player attempted to be added to players while valid player was still there\n");
        critical::invalidatePlayer(players, client_id, dataMutex);
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
        return;
    }

    // TODO: If Hosting, create a 'lobby' in which they can play games.
    // Lobby can only have the two players, but do this so games can be repeated and
    // players can swap being the red and blue player

    if (isHosting)
    {
        Lobby client_lobby = Lobby(client_player);
        message_sent_success = matchmaking::reportSuccessfulLobbyCreation(client_fd);
        if (!message_sent_success)
        {
            std::print(stderr, "Error: message send unsucessful\n");
            critical::invalidatePlayer(players, client_id, dataMutex);
            critical::addIDToQueue(freeIDs, client_id, dataMutex);
            networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
            return;
        }

        const bool lobbyAdded = critical::addLobbyToLobbies(lobbies, client_lobby, dataMutex);
        if (!lobbyAdded)
        {
            std::print(stderr, "Error: lobby attempted to be added to lobbies while valid lobby was still there\n");
            critical::invalidatePlayer(players, client_id, dataMutex);
            critical::addIDToQueue(freeIDs, client_id, dataMutex);
            networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
            return;
        }

        // Wait until someone joins the lobby
        Player guest;
        while (true)
        {
            // Don't bother checking atomically until there's a sign that someone joined. Checking atomically would constantly block every thread.
            if (client_lobby.m_guest.m_isValid)
            {
                guest = critical::getGuestFromClientLobby(lobbies, client_id, dataMutex);
                if (guest.m_isValid)
                {
                    break;
                }
            }
        }

        message_sent_success = matchmaking::sendHostTheGuestName(client_fd, guest.m_name);
        if (!message_sent_success)
        {
            std::print(stderr, "Error: message send unsucessful\n");
            critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
            critical::invalidatePlayerOnceLobbyIsInvalid(players, lobbies, client_id, dataMutex);
            critical::addIDToQueue(freeIDs, client_id, dataMutex);
            networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
            return;
        }

        auto [hostPickedRed, disconnectedTmp1] = matchmaking::hostChoosesRed(client_fd);
        client_disconnected = disconnectedTmp1;
        if (client_disconnected)
        {
            critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
            critical::invalidatePlayerOnceLobbyIsInvalid(players, lobbies, client_id, dataMutex);
            critical::addIDToQueue(freeIDs, client_id, dataMutex);
            networking::closeFd(client_fd);
            return;
        }

        // TODO: have some sort of code that confirms if your opponent doesn't want to play anymore (or disconnected)
        bool wantToPlay = true;
        while (wantToPlay)
        {
            // TODO: move this to the not hosting code. Have them check the lobby.someoneDisconnected and the gamestate.isvalid before doing this
            message_sent_success = matchmaking::sendGuestTheHostColor(client_fd, hostPickedRed); // TODO: change to lobby host's fd
            if (!message_sent_success)
            {
                std::print(stderr, "Error: message send unsucessful\n");
                critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
                critical::invalidatePlayerOnceLobbyIsInvalid(players, lobbies, client_id, dataMutex);
                critical::addIDToQueue(freeIDs, client_id, dataMutex);
                networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
                return;
            }
            // TODO: end code to move

            GameState gamestate;
            if (hostPickedRed)
            {
                gameMutexes[client_id].lock(); // NOTE: make sure that guest stalls until gamestate is added (or disconnect) to try to get the lock
                gamestate = GameState(client_player, guest);
            }
            else
            {
                gamestate = GameState(guest, client_player);
            }
            const bool gamestateAdded = critical::addGameStateToGameStates(gamestates, gamestate, client_id, dataMutex);
            if (!gamestateAdded)
            {
                std::print(stderr, "Error: gamestate attempted to be added to gamestates while valid gamestate was still there\n");
                if (hostPickedRed)
                {
                    gameMutexes[client_id].unlock();
                }
                critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
                critical::invalidatePlayerOnceLobbyIsInvalid(players, lobbies, client_id, dataMutex);
                critical::addIDToQueue(freeIDs, client_id, dataMutex);
                networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
                return;
            }

            auto [wantToContinue, disconnectedTmp2, oppDisconnected] = play::playGame(hostPickedRed, client_id, client_fd, gamestates, gameMutexes);
            client_disconnected = disconnectedTmp2;
            wantToPlay = wantToContinue;
            if (client_disconnected || !wantToPlay)
            {
                // TODO: either make sure you don't have the lock or you free it here
                critical::invalidateGamestateIfOtherPlayerDisconnected(gamestates, client_id, dataMutex, disconnectMutex);
                critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
                critical::invalidatePlayerOnceLobbyIsInvalid(players, lobbies, client_id, dataMutex);
                critical::addIDToQueue(freeIDs, client_id, dataMutex);
                networking::closeFd(client_fd);
                return;
            }

            // TODO: Paths:
            // 1. Doesn't want to play anymore: clean up the thread. Can probably add that to the above cleanup
            // 2. Wants to play but opp disconnected or doesn't: put back into loop where client is waiting for connection.
            //      Make sure to reset the gamestate and lobby and stuff to what they would be at the beginning of that loop.
            // 3. Wants to play and opp does want to play: reset gamestate and lobby to what is correct for that scenario

            // TODO: Check if opp disconnected or doesn't want to play anymore so you know to go back to waiting for a player
            if (oppDisconnected)
            {
                // TODO: handle going back to code where host waits for opponent
            }
            // TODO: make sure gamestate gets cleaned up and reset or something but not before opp is finished

        }
    }
    else // client wants to join existing lobby
    {
        // TODO:
        // Read list of lobbies and send to client.
        // Expect client to either disconnect, try to join a lobby (which may or may not work if the lobby is now full or doesn't exist), or refresh the list.
        // Send client their assigned color from the host
        // Do the game logic
    }

    critical::addIDToQueue(freeIDs, client_id, dataMutex);
    networking::closeFd(client_fd);
}

int main()
{

    int serv_fd = networking::initServer();
    std::mutex dataMutex;
    std::mutex disconnectMutex;

    // arraySize defined in settings.hpp
    std::array<Player, arraySize> players;
    std::array<GameState, arraySize> gameStates;
    std::array<Lobby, arraySize> lobbies;
    std::array<std::mutex, arraySize> gameMutexes;

    std::queue<std::uint8_t> freeIDs = std::queue<std::uint8_t>();
    initializeFreeIDs(freeIDs, arraySize);

    while (true)
    {
        int client_fd = networking::acceptConnection(serv_fd);
        if (client_fd == -1)
        {
            continue;
        }
        std::thread clientThread(manageClient, client_fd, std::ref(players), std::ref(gameStates), std::ref(lobbies), std::ref(freeIDs), std::ref(dataMutex), std::ref(disconnectMutex), std::ref(gameMutexes));
        clientThread.detach();
    }

    networking::closeFd(serv_fd);

    return 0;
}
