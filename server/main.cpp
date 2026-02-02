#include "GameState.hpp"
#include "Lobby.hpp"
#include "Player.hpp"
#include "critical.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"
#include "winner.hpp"

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

GameState updateGamestate(bool redMove, std::uint8_t location, GameState previousState)
{
    return GameState();
}

std::tuple<bool, bool> playGame(bool isRed, std::uint8_t hostID, int client_fd, std::array<GameState, arraySize>& gamestates, std::array<Lobby, arraySize>& lobbies, std::mutex& dataMutex, std::array<std::mutex, arraySize>& gameMutexes)
{
    /*
     * Returns [wantsToPlayAgain: bool, disconnected: bool]
     */

    /*
     * Red: has lock before getting in this function. The initial state at the start of its loop should be the same as its state, but only that time
     * Blue: should not have lock before this function. Checks its state against initial state until it changes then it can take the lock
     */

    bool isFirstTurn = true;
    GameState gamestate = gamestates[hostID];

    while (true)
    {
        if (!isFirstTurn) // skip the board progress checking if it's the 1st turn and you're red
        {
            // TODO: probably add check for if opponent disconnected and make sure the state isn't the same as when you finished the loop (making sure you didn't skip their lock)
            // NOTE: if you're here you should have the lock
            bool yourTurnNow = false;
            while (!yourTurnNow)
            {
                // Wait if the opponent hasn't taken their turn yet
                if (gamestate == gamestates[hostID]) // TODO: add code to check if opponent disconnected
                {
                    gameMutexes[hostID].unlock();
                    std::this_thread::yield();
                    gameMutexes[hostID].lock();
                }
                else
                {
                    yourTurnNow = true;
                }
            }
        }
        else
        {
            if (!isRed) // if you're blue and it's first turn, wait while gamestate is in initial state then take lock
            {
                bool redDidFirstTurn = false;
                while (!redDidFirstTurn) // TODO: add code to check if red disconnected
                {
                    gameMutexes[hostID].lock();
                    redDidFirstTurn = gamestates[hostID].isInitialState();
                    gameMutexes[hostID].unlock();
                }
                gameMutexes[hostID].lock();
            }
        }

        // Check to see if your opponent already won/stalemated
        gamestate = gamestates[hostID];
        std::uint8_t theWinner = winner::winner(gamestate.m_board);
        if (theWinner != 0)
        {
            // TODO: send msg of success or failure
            gameMutexes[hostID].unlock();
            break;
        }
        else
        {
            // TODO: send msg of continuing game
        }

        // Get your move
        bool message_sent_success = matchmaking::sendBoardState(client_fd, gamestates[hostID].m_board);
        if (!message_sent_success)
        {
            std::print(stderr, "Error: message send unsucessful\n");
            gameMutexes[hostID].unlock();
            return std::make_tuple(false, true);
        }

        auto [move, disconnectedTmp0] = matchmaking::getClientMove(client_fd);
        bool client_disconnected = disconnectedTmp0;
        if (client_disconnected)
        {
            gameMutexes[hostID].unlock();
            return std::make_tuple(false, true);
        }

        // Update gamestate and see if you win/stalemate
        gamestate = updateGamestate(true, move, gamestate);
        gamestates[hostID] = gamestate;
        theWinner = winner::winner(gamestate.m_board);
        if (theWinner != 0)
        {
            // TODO: send msg of success or failure
            gameMutexes[hostID].unlock();
            break;
        }
        else
        {
            // TODO: send msg of continuing game
        }
        isFirstTurn = false;

        // Let the other player get their turn
        gameMutexes[hostID].unlock();
        std::this_thread::yield();
        gameMutexes[hostID].lock();
    }
    // TODO: receive msg of whether they want to play again

    return std::make_tuple(false, true);
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
            // TODO: move this to the not hosting code. Have them check the lobby.someoneDisconnected and the gamestate.isvalid before doing this
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
            // TODO: end code to move

            if (hostPickedRed)
            {
                gameMutexes[client_id].lock(); // NOTE: make sure that guest stalls until gamestate is added (or disconnect) to try to get the lock
                GameState gamestate = GameState(client_player, guest);
                const bool gamestateAdded = critical::addGameStateToGameStates(gamestates, gamestate, client_id, dataMutex);
                if (!gamestateAdded)
                {
                    std::print(stderr, "Error: gamestate attempted to be added to gamestates while valid gamestate was still there\n");
                    gameMutexes[client_id].unlock();
                    critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
                    critical::invalidatePlayer(players, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
                    return;
                }

                auto [wantToContinue, disconnectedTmp2] = playGame(true, client_id, client_fd, gamestates, lobbies, dataMutex, gameMutexes);
                client_disconnected = disconnectedTmp2;
                if (client_disconnected)
                {
                    critical::invalidateGamestateIfOtherPlayerDisconnected(gamestates, lobbies, client_id, dataMutex, disconnectMutex);
                    critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
                    critical::invalidatePlayer(players, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd);
                    return;
                }

                wantToPlay = wantToContinue;
            }
            else // lobby owner picked blue
            {
                GameState gamestate = GameState(guest, client_player);
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

                auto [wantToContinue, disconnectedTmp2] = playGame(false, guest.m_id, client_fd, gamestates, lobbies, dataMutex, gameMutexes);
                client_disconnected = disconnectedTmp2;
                if (client_disconnected)
                {
                    // TODO: either make sure you don't have the lock or you free it here
                    critical::invalidateGamestateIfOtherPlayerDisconnected(gamestates, lobbies, client_id, dataMutex, disconnectMutex);
                    critical::closeLobbyIfOtherPlayerDisconnected(lobbies, client_lobby, dataMutex, disconnectMutex);
                    critical::invalidatePlayer(players, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd);
                    return;
                }

                wantToPlay = wantToContinue;
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
