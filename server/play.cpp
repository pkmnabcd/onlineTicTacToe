#include "play.hpp"

#include "GameState.hpp"
#include "Lobby.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <mutex>
#include <print>
#include <string>
#include <thread>
#include <tuple>


std::uint8_t horizontalWinner(StraightBoard board)
{
    /*
     * Checks for and returns the winner in the horizontal direction
     * If no winner, 0 is returned
    */
    // Check first row
    if (board[0] == board[1] && board[0] == board[2])
    {
        return board[0];  // Assume cell has single char, X or O
    }
    else if (board[3] == board[4] && board[3] == board[5])
    {
        return board[3];
    }
    else if (board[6] == board[7] && board[6] == board[8])
    {
        return board[6];
    }
    else
    {
        return 0;
    }
}

std::uint8_t verticalWinner(StraightBoard board)
{
    /*
     * Checks for and returns the winner in the vertical direction
     * If no winner, 0 is returned
    */
    // Check first col
    if (board[0] == board[3] && board[0] == board[6])
    {
        return board[0];  // Assume cell has single char, X or O
    }
    else if (board[1] == board[4] && board[1] == board[7])
    {
        return board[1];
    }
    else if (board[2] == board[5] && board[2] == board[8])
    {
        return board[2];
    }
    else
    {
        return 0;
    }
}

std::uint8_t diagonalWinner(StraightBoard board)
{
    /*
     * Checks for and returns the winner in the diagonal direction
     * If no winner, 0 is returned
    */
    // Check first col
    if (board[0] == board[4] && board[0] == board[8])
    {
        return board[4];  // Assume cell has single char, X or O
    }
    else if (board[2] == board[4] && board[2] == board[6])
    {
        return board[4];
    }
    else
    {
        return 0;
    }
}

bool boardFull(StraightBoard board)
{
    for (std::uint8_t i = 0; i < 9; i++)
    {
        if (board[i] != 'X' && board[i] != 'O')
        {
            return false;
        }
    }
    return true;
}

std::uint8_t winner(StraightBoard board)
{
    /*
     * Checks for and returns the winner
     * If no winner, 0 is returned
    */

    if (std::uint8_t horWinner = horizontalWinner(board); horWinner)
    {
        return horWinner;
    }
    else if (std::uint8_t verWinner = verticalWinner(board); verWinner)
    {
        return verWinner;
    }
    else if (std::uint8_t diaWinner = diagonalWinner(board); diaWinner)
    {
        return diaWinner;
    }
    else if (boardFull(board))
    {
        return 'S';
    }
    else
    {
        return 0;
    }
}

std::tuple<GameState, bool> updateGamestate(bool redMove, std::uint8_t location, GameState previousState)
{
    /*
     * Returns [updatedGamestate: GameState, invalid: bool]
     */

    using StraightBoard = std::array<char, 9>;
    char newChar = (redMove) ? 'X' : 'O'; // X is red, O is blue

    GameState gamestate = previousState;
    StraightBoard board = gamestate.m_board;
    char val = board[location - 1]; // location is 1-indexed while arrays are 0-indexed

    if (val - 48 == location) // What's supposed to happen
    {
        board[location - 1] = newChar;
        gamestate.m_board = board;
        return std::make_tuple(gamestate, false);
    }
    else // This means there is something in the spot already
    {
        return std::make_tuple(GameState(), true);
    }
}

std::tuple<bool, bool> waitTurn(bool isFirstTurn, GameState gamestate, bool isRed, std::uint8_t hostID, SocketType client_fd, std::array<GameState, arraySize>& gamestates, std::array<Lobby, arraySize>& lobbies, std::array<std::mutex, arraySize>& gameMutexes)
{
    /*
     * This function waits until your opponent moves or disconnects
     * Returns [client_disconnected: bool, oppDisconnected: bool]
     */

    // NOTE: if you're here you should have the lock

    // Figure out what function you use to decide if you wait.
    // Returning true means you should wait. False means you're done waiting.
    std::function<bool(const GameState&, const GameState&)> waitFunc;
    if (isFirstTurn && !isRed) // first turn and blue
    {
        waitFunc = []( [[maybe_unused]]const GameState& savedState, const GameState& updatedState) -> bool { return updatedState.isInitialState(); };
    }
    else if (isFirstTurn) // first turn and red
    {
        waitFunc = []( [[maybe_unused]]const GameState& savedState, [[maybe_unused]]const GameState& updatedState) -> bool { return false; };
    }
    else // not first turn
    {
        waitFunc = [](const GameState& savedState, const GameState& updatedState) -> bool { return savedState == updatedState; };
    }

    bool message_sent_success = false;
    bool oppDisconnected = false;

    // Wait until the opponent takes their turn
    while (waitFunc(gamestate, gamestates[hostID]))
    {
        gameMutexes[hostID].unlock();
        std::this_thread::yield();
        gameMutexes[hostID].lock();
        if (lobbies[hostID].m_someoneDisconnected)
        {
            message_sent_success = matchmaking::sendClientGameStatus(client_fd, 'D');
            if (!message_sent_success)
            {
                return std::make_tuple(true, true);
            }
            oppDisconnected = true;
            break; // out of wait loop
        }
    }

    return std::make_tuple(false, oppDisconnected);
}

std::tuple<bool, bool, bool> play::playGame(bool isRed, std::uint8_t hostID, SocketType client_fd, std::array<GameState, arraySize>& gamestates, std::array<Lobby, arraySize>& lobbies, std::array<std::mutex, arraySize>& gameMutexes)
{
    /*
     * Returns [wantsToPlayAgain: bool, disconnected: bool, oppDisconnected: bool]
     */

    gameMutexes[hostID].lock();

    bool isFirstTurn = true;
    bool message_sent_success = false;
    bool oppDisconnected = false;
    bool client_disconnected = false;;
    GameState gamestate = gamestates[hostID];

    while (true)
    {
        // Wait for your turn to move
        auto [client_disconnectedTmp0, oppDisconnectedTmp] = waitTurn(isFirstTurn, gamestate, isRed, hostID, client_fd, gamestates, lobbies, gameMutexes);
        oppDisconnected = oppDisconnectedTmp;
        client_disconnected = client_disconnectedTmp0;
        if (client_disconnected)
        {
            std::print(stderr, "Error: message send unsucessful\n");
            gameMutexes[hostID].unlock();
            return std::make_tuple(false, client_disconnected, oppDisconnected);
        }
        if (oppDisconnected)
        {
            // NOTE: client already got sent the 'D' game status
            gameMutexes[hostID].unlock();
            break; // out of game loop
        }

        // Check to see if your opponent already won/stalemated
        gamestate = gamestates[hostID];
        std::uint8_t theWinner = winner(gamestate.m_board);
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

        // Send client the board state
        message_sent_success = matchmaking::sendBoardState(client_fd, gamestates[hostID].m_board);
        if (!message_sent_success)
        {
            std::print(stderr, "Error: message send unsucessful\n");
            gameMutexes[hostID].unlock();
            return std::make_tuple(false, true, false);
        }
        // If game finished, break out of game loop
        if (theWinner != 0)
        {
            gameMutexes[hostID].unlock();
            break;
        }

        // Get your move
        auto [move, clientQuit, disconnectedTmp0] = matchmaking::getClientMove(client_fd);
        client_disconnected = disconnectedTmp0;
        if (client_disconnected)
        {
            gameMutexes[hostID].unlock();
            return std::make_tuple(false, true, false);
        }
        if (clientQuit)
        {
            return std::make_tuple(false, false, false);
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
        theWinner = winner(gamestate.m_board);
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

    // NOTE: make sure that game lock is unlocked when getting here
    gamestates[hostID].m_gameFinished = true;
    auto [playAgain, client_disconnectedTmp1] = matchmaking::getClientPlayAgain(client_fd);
    client_disconnected = client_disconnectedTmp1;
    if (client_disconnected)
    {
        std::print(stderr, "Error: bad playAgain detected\n");
        return std::make_tuple(false, true, oppDisconnected);
    }

    return std::make_tuple(playAgain, client_disconnected, oppDisconnected);
}
