#include "play.hpp"

#include "interface.hpp"
#include "matchmaking.hpp"
#include "util.hpp"

#include <cstdint>
#include <format>
#include <print>
#include <string>
#include <tuple>


std::tuple<bool, bool, bool> play::playGame(int serv_fd, bool isRed, std::string oppName)
{
    /*
     * Returns [wantsToPlayAgain: bool, disconnected: bool, oppDisconnected: bool]
     */
    bool wantsToPlayAgain = false;
    bool disconnected = false;
    bool oppDisconnected = false;

    std::string yourLetter = (isRed) ? "X" : "O";
    bool firstTurn = true;
    Board previousBoard = util::makeBoard();
    Board currentBoard;
    while (true) // game loop
    {
        // Print the initial board if you're blue while you're waiting
        if (!isRed && firstTurn)
        {
            interface::show(previousBoard);
        }

        // Get existing state
        std::print("Waiting for the status update!\n");
        auto [continuePlay, winner, oppDisconnectedTmp0, disconnectedTmp0] = matchmaking::getGameStatus(serv_fd);
        oppDisconnected = oppDisconnectedTmp0;
        disconnected = disconnectedTmp0;
        if (disconnected)
        {
            std::print("Got disconnected while waiting for gamestate update.\n");
            return std::make_tuple(wantsToPlayAgain, disconnected, oppDisconnected);
        }
        if (oppDisconnected)
        {
            std::print("Your opponent disconnected. Ending game.\n");
            return std::make_tuple(wantsToPlayAgain, disconnected, oppDisconnected);
        }

        // Get existing board
        std::print("Waiting for the board state!\n");
        auto [currentBoardTmp0, disconnectedTmp1] = matchmaking::getBoardState(serv_fd);
        currentBoard = currentBoardTmp0;
        disconnected = disconnectedTmp1;
        if (disconnected)
        {
            std::print("Got disconnected while waiting for board update.\n");
            return std::make_tuple(wantsToPlayAgain, disconnected, oppDisconnected);
        }
        std::print("Got the board state!\n");
        bool skipOppPrint = isRed && firstTurn; // Print opponent move unless you're first move
        if (!skipOppPrint)
        {
            std::uint8_t pos = util::getMovePosition(previousBoard, currentBoard);
            interface::printOppTurnMessage(pos, !isRed, oppName);
        }

        // If the game is over, show the final move and break out of game loop
        if (!continuePlay)
        {
            interface::printWinnerMessage(winner);
            interface::show(currentBoard);
            break; // Go to code that asks whether to play again.
        }

        // Get move from user and send to server
        std::int8_t yourMove = interface::getHumanMove(currentBoard, yourLetter);


        firstTurn = false;
        previousBoard = currentBoard;
    }
    // TODO: Process for this function:
    // 1. get the game status
    // 2. get the board state
    // 3. get move from user and send to server
    // 4. get game status again
    // 5. Repeat until status other than 'C'
    // 6. Decide if you want to play again
    return std::make_tuple(true, true, true);
}
