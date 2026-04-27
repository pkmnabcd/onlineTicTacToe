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

std::tuple<GameState, bool> updateGamestate(bool redMove, std::uint8_t location, GameState previousState)
{
    /*
     * Returns [updatedGamestate: GameState, invalid: bool]
     */

    using StraightBoard = std::array<std::string, 9>;
    char newChar = (redMove) ? 'X' : 'O'; // X is red, O is blue

    GameState gamestate = previousState;
    StraightBoard board = gamestate.m_board;
    std::string str = board[location - 1]; // location is 1-indexed while arrays are 0-indexed
    char val = str[0];

    if (val - 48 == location) // What's supposed to happen
    {
        board[location - 1] = std::string(1, newChar);
        gamestate.m_board = board;
        return std::make_tuple(gamestate, false);
    }
    else // This means there is something in the spot already
    {
        return std::make_tuple(GameState(), true);
    }
}

std::tuple<bool, bool, bool> playGame(bool isRed, std::uint8_t hostID, int client_fd, std::array<GameState, arraySize>& gamestates, std::array<std::mutex, arraySize>& gameMutexes)
{
    /*
     * Returns [wantsToPlayAgain: bool, disconnected: bool, oppDisconnected: bool]
     */

    /*
     * Red: has lock before getting in this function. The initial state at the start of its loop should be the same as its state, but only that time
     * Blue: should not have lock before this function. Checks its state against initial state until it changes then it can take the lock
     */

    bool isFirstTurn = true;
    bool message_sent_success = false;
    bool oppDisconnected = false;
    GameState gamestate = gamestates[hostID];

    while (true)
    {
        // NOTE: this whole if-else is just to make sure threads are blocked so turn order is correct.
        if (!isFirstTurn) // skip the board progress checking if it's the 1st turn and you're red
        {
            // NOTE: if you're here you should have the lock
            bool yourTurnNow = false;
            while (!yourTurnNow)
            {
                // Wait if the opponent hasn't taken their turn yet
                if (gamestate == gamestates[hostID])
                {
                    gameMutexes[hostID].unlock();
                    std::this_thread::yield();
                    gameMutexes[hostID].lock();
                    if (gamestates[hostID].m_someoneDisconnected)
                    {
                        message_sent_success = matchmaking::sendClientGameStatus(client_fd, 'D');
                        if (!message_sent_success)
                        {
                            std::print(stderr, "Error: message send unsucessful\n");
                            gameMutexes[hostID].unlock();
                            return std::make_tuple(false, true, true);
                        }
                        oppDisconnected = true;
                        break; // out of wait loop
                    }
                }
                else
                {
                    yourTurnNow = true;
                }
            }
            if (oppDisconnected)
            {
                break; // out of game loop
            }
        }
        else // Is the first turn
        {
            if (!isRed) // if you're blue and it's first turn, wait while gamestate is in initial state then take lock to take your turn
            {
                bool waitingForRed = true;
                while (waitingForRed)
                {
                    gameMutexes[hostID].lock();
                    if (gamestates[hostID].m_someoneDisconnected)
                    {
                        message_sent_success = matchmaking::sendClientGameStatus(client_fd, 'D');
                        if (!message_sent_success)
                        {
                            std::print(stderr, "Error: message send unsucessful\n");
                            gameMutexes[hostID].unlock();
                            return std::make_tuple(false, true, true);
                        }
                        oppDisconnected = true;
                        break; // out of wait loop
                    }
                    waitingForRed = gamestates[hostID].isInitialState();
                    if (waitingForRed)
                    {
                        gameMutexes[hostID].unlock();
                    }
                }
                if (oppDisconnected)
                {
                    // TODO: this seems to mean that player is unlocked when exiting the loop.
                    // Figure out if we want to unlock when exiting correctly or not.
                    // I'm thinking yes right now.
                    gameMutexes[hostID].unlock();
                    break; // out of game loop
                }
            }
        }

        // Check to see if your opponent already won/stalemated
        gamestate = gamestates[hostID];
        std::uint8_t theWinner = winner::winner(gamestate.m_board);
        if (theWinner != 0)
        {
            // send msg of success or failure
            message_sent_success = matchmaking::sendClientGameStatus(client_fd, static_cast<char>(theWinner));
            if (!message_sent_success)
            {
                std::print(stderr, "Error: message send unsucessful\n");
                gameMutexes[hostID].unlock();
                return std::make_tuple(false, true, false);
            }
            // TODO: this seems to mean that player is unlocked when winner is found.
            // Figure out if we want to unlock when exiting correctly or not.
            // I'm thinking yes right now.
            gameMutexes[hostID].unlock();
            break;
        }
        else
        {
            // send msg of continuing game
            message_sent_success = matchmaking::sendClientGameStatus(client_fd, 'C');
            if (!message_sent_success)
            {
                std::print(stderr, "Error: message send unsucessful\n");
                gameMutexes[hostID].unlock();
                return std::make_tuple(false, true, false);
            }
        }

        // Get your move
        message_sent_success = matchmaking::sendBoardState(client_fd, gamestates[hostID].m_board);
        if (!message_sent_success)
        {
            std::print(stderr, "Error: message send unsucessful\n");
            gameMutexes[hostID].unlock();
            return std::make_tuple(false, true, false);
        }

        auto [move, disconnectedTmp0] = matchmaking::getClientMove(client_fd);
        bool client_disconnected = disconnectedTmp0;
        if (client_disconnected)
        {
            gameMutexes[hostID].unlock();
            return std::make_tuple(false, true, false);
        }

        // Update gamestate and see if you win/stalemate
        auto [newGamestate, invalidMove] = updateGamestate(isRed, move, gamestate);
        gamestate = newGamestate;
        if (invalidMove)
        {
            std::print(stderr, "Error: bad move detected\n");
            gameMutexes[hostID].unlock();
            return std::make_tuple(false, true, false);
        }

        gamestates[hostID] = gamestate;
        theWinner = winner::winner(gamestate.m_board);
        if (theWinner != 0)
        {
            // send msg of success or failure
            message_sent_success = matchmaking::sendClientGameStatus(client_fd, static_cast<char>(theWinner));
            if (!message_sent_success)
            {
                std::print(stderr, "Error: message send unsucessful\n");
                gameMutexes[hostID].unlock();
                return std::make_tuple(false, true, false);
            }
            // TODO: this seems to mean that player is unlocked when winner is found.
            // Figure out if we want to unlock when exiting correctly or not.
            // I'm thinking yes right now.
            gameMutexes[hostID].unlock();
            break;
        }
        else
        {
            // send msg of continuing game
            message_sent_success = matchmaking::sendClientGameStatus(client_fd, 'C');
            if (!message_sent_success)
            {
                std::print(stderr, "Error: message send unsucessful\n");
                gameMutexes[hostID].unlock();
                return std::make_tuple(false, true, false);
            }
        }
        isFirstTurn = false;

        // Let the other player get their turn
        gameMutexes[hostID].unlock();
        std::this_thread::yield();
        gameMutexes[hostID].lock();
    }

    auto [playAgain, client_disconnected] = matchmaking::getClientPlayAgain(client_fd);
    if (client_disconnected)
    {
        std::print(stderr, "Error: bad playAgain detected\n");
        gameMutexes[hostID].unlock();
        return std::make_tuple(false, true, oppDisconnected);
    }

    return std::make_tuple(playAgain, client_disconnected, oppDisconnected);
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

            auto [wantToContinue, disconnectedTmp2, oppDisconnected] = playGame(hostPickedRed, client_id, client_fd, gamestates, gameMutexes);
            client_disconnected = disconnectedTmp2;
            if (client_disconnected)
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

            wantToPlay = wantToContinue;
            // TODO: If doesn't want to play, make sure lobby gets cleaned up
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
