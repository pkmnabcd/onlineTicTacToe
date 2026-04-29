#include "play.hpp"

#include "GameState.hpp"
#include "matchmaking.hpp"

#include <array>
#include <cstdint>
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
        return board[0][0];  // Assume cell has single char, X or O
    }
    else if (board[3] == board[4] && board[3] == board[5])
    {
        return board[3][0];
    }
    else if (board[6] == board[7] && board[6] == board[8])
    {
        return board[6][0];
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
        return board[0][0];  // Assume cell has single char, X or O
    }
    else if (board[1] == board[4] && board[1] == board[7])
    {
        return board[1][0];
    }
    else if (board[2] == board[5] && board[2] == board[6])
    {
        return board[2][0];
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
        return board[4][0];  // Assume cell has single char, X or O
    }
    else if (board[2] == board[4] && board[2] == board[6])
    {
        return board[4][0];
    }
    else
    {
        return 0;
    }
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

std::tuple<bool, bool, bool> play::playGame(bool isRed, std::uint8_t hostID, int client_fd, std::array<GameState, arraySize>& gamestates, std::array<std::mutex, arraySize>& gameMutexes)
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
            // TODO: maybe make the waiting loops into a function?
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
                gameMutexes[hostID].unlock();
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
                    gameMutexes[hostID].unlock();
                    break; // out of game loop
                }
            }
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
    auto [playAgain, client_disconnected] = matchmaking::getClientPlayAgain(client_fd);
    if (client_disconnected)
    {
        std::print(stderr, "Error: bad playAgain detected\n");
        return std::make_tuple(false, true, oppDisconnected);
    }

    return std::make_tuple(playAgain, client_disconnected, oppDisconnected);
}
