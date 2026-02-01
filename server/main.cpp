#include "GameState.hpp"
#include "Lobby.hpp"
#include "Player.hpp"
#include "critical.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"

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

std::tuple<GameState, bool> updateGamestateAndCheckForWinner(bool redMove, std::uint8_t location, GameState previousState)
{
    return std::make_tuple(GameState(), true);
}

void manageClient(int client_fd, std::array<Player, arraySize>& players, std::array<GameState, arraySize>& gamestates, std::array<Lobby, arraySize>& lobbies, std::queue<std::uint8_t>& freeIDs, std::mutex& dataMutex, std::mutex& disconnectMutex)
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
        while (true)
        {
            if (client_lobby.m_guest.m_isValidPlayer)
            {
                // Don't bother checking atomically until there's a sign that someone joined. Checking atomically would slow every thread way down.
                break;
            }
        }
        // TODO: it's possible that I'll need to check the m_isValidPlayer again atomically just in case
        Player guest = critical::getGuestFromClientLobby(lobbies, client_id, dataMutex);

        message_sent_success = matchmaking::sendHostTheGuestName(client_fd, guest.m_name);
        if (!message_sent_success)
        {
            std::print(stderr, "Error: message send unsucessful\n");
            critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
            critical::invalidatePlayer(players, client_id, dataMutex);
            critical::addIDToQueue(freeIDs, client_id, dataMutex);
            networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
            return;
        }

        auto [hostPickedRed, disconnectedTmp1] = matchmaking::hostChoosesRed(client_fd);
        client_disconnected = disconnectedTmp1;
        if (client_disconnected)
        {
            critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
            critical::invalidatePlayer(players, client_id, dataMutex);
            critical::addIDToQueue(freeIDs, client_id, dataMutex);
            networking::closeFd(client_fd);
            return;
        }

        bool wantToPlay = true;
        while (wantToPlay)
        {
            message_sent_success = matchmaking::sendGuestTheHostColor(client_fd, hostPickedRed);
            if (!message_sent_success)
            {
                std::print(stderr, "Error: message send unsucessful\n");
                critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
                critical::invalidatePlayer(players, client_id, dataMutex);
                critical::addIDToQueue(freeIDs, client_id, dataMutex);
                networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
                return;
            }

            if (hostPickedRed)
            {
                GameState gamestate = GameState(client_player, guest);
                const bool gamestateAdded = critical::addGameStateToGameStates(gamestates, gamestate, client_id, dataMutex);
                if (!gamestateAdded)
                {
                    std::print(stderr, "Error: gamestate attempted to be added to gamestates while valid gamestate was still there\n");
                    critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
                    critical::invalidatePlayer(players, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
                    return;
                }

                message_sent_success = matchmaking::sendBoardState(client_fd, gamestates[client_id].m_board);
                if (!message_sent_success)
                {
                    std::print(stderr, "Error: message send unsucessful\n");
                    critical::invalidateGamestate(gamestates, client_id, dataMutex);
                    critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
                    critical::invalidatePlayer(players, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
                    return;
                }

                auto [hostMove, disconnectedTmp2] = matchmaking::getClientMove(client_fd);
                client_disconnected = disconnectedTmp2;
                if (client_disconnected)
                {
                    critical::invalidateGamestate(gamestates, client_id, dataMutex); // TODO: might need similar to lobby where I check if other player disconnected first. Also may want to use disconnect mutex for that
                    critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
                    critical::invalidatePlayer(players, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd);
                    return;
                }
                // TODO: figure out a way for one thread to know that the other client has disconnected.
                // Maybe access the lobby m_someoneDisconnected member atomically and see if the other thread has already changed it,
                // then you can either clean up the lobby or leave that to the other thread.

                // For normal synchronization it should be good enough to just have one thread wait to receive the new state from the server
                // while the other thread considers the input it will give to the server.
                // Then they will just check each time if the other player is still there.
                // So each thread will compare their copy of gamestate with the one in gamestates[] and once it changes, they know they can go
            }
            else
            {
            }
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

    // TODO: If joining, give a list of lobbies that need a second player.

    critical::addIDToQueue(freeIDs, client_id, dataMutex);
    networking::closeFd(client_fd);
}

int main()
{

    int serv_fd = networking::initServer();
    std::mutex dataMutex;
    std::mutex disconnectMutex;

    const std::size_t arraySize = static_cast<std::size_t>(UINT8_MAX) + 1;
    std::array<Player, arraySize> players;
    std::array<GameState, arraySize> gameStates;
    std::array<Lobby, arraySize> lobbies;

    std::queue<std::uint8_t> freeIDs = std::queue<std::uint8_t>();
    initializeFreeIDs(freeIDs, arraySize);

    while (true)
    {
        int client_fd = networking::acceptConnection(serv_fd);
        if (client_fd == -1)
        {
            continue;
        }
        std::thread clientThread(manageClient, client_fd, std::ref(players), std::ref(gameStates), std::ref(lobbies), std::ref(freeIDs), std::ref(dataMutex), std::ref(disconnectMutex));
        clientThread.detach();
    }

    networking::closeFd(serv_fd);

    return 0;
}
