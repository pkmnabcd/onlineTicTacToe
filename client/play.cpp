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

        if (!firstTurn || !isRed)
        {
            std::print("Waiting for opponent to move.\n\n");
        }
        // Get existing state
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
            break;
        }

        // Get existing board
        auto [currentBoardTmp0, disconnectedTmp1] = matchmaking::getBoardState(serv_fd);
        currentBoard = currentBoardTmp0;
        disconnected = disconnectedTmp1;
        if (disconnected)
        {
            std::print("Got disconnected while waiting for board update.\n");
            return std::make_tuple(wantsToPlayAgain, disconnected, oppDisconnected);
        }
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

        // Get move from user
        std::int8_t yourMove = interface::getHumanMove(currentBoard, yourLetter);
        if (yourMove == -1) // quit
        {
            wantsToPlayAgain = false;
            return std::make_tuple(wantsToPlayAgain, disconnected, oppDisconnected);
        }

        // Apply the move and show it
        StraightBoard tmpBoard = util::get1dFrom2dBoard(currentBoard);
        tmpBoard[yourMove - 1] = yourLetter;
        currentBoard = matchmaking::getBoardFromStraightBoard(tmpBoard);
        interface::show(currentBoard);

        // Send move to server
        bool sentMove = matchmaking::sendMove(serv_fd, yourMove);
        disconnected = !sentMove;
        if (disconnected)
        {
            std::print("Got disconnected while sending your move.\n");
            return std::make_tuple(wantsToPlayAgain, disconnected, oppDisconnected);
        }

        // Get back whether to expect another turn or not.
        auto [continuePlayTmp0, winnerTmp0, oppDisconnectedTmp1, disconnectedTmp2] = matchmaking::getGameStatus(serv_fd);
        continuePlay = continuePlayTmp0;
        winner = winnerTmp0;
        oppDisconnected = oppDisconnectedTmp1;
        disconnected = disconnectedTmp2;
        if (disconnected)
        {
            std::print("Got disconnected while waiting for gamestate update.\n");
            return std::make_tuple(wantsToPlayAgain, disconnected, oppDisconnected);
        }

        // If the game is over, show the final move and break out of game loop
        if (!continuePlay)
        {
            interface::printWinnerMessage(winner);
            interface::show(currentBoard);
            break; // Go to code that asks whether to play again.
        }

        firstTurn = false;
        previousBoard = currentBoard;
    }
    // Decide if play again
    if (!oppDisconnected)
    {
        wantsToPlayAgain = interface::playAgain(oppName);
    }
    else
    {
        wantsToPlayAgain = true;
    }
    bool message_sent_success = matchmaking::sendPlayAgain(serv_fd, wantsToPlayAgain);
    disconnected = !message_sent_success;
    if (disconnected)
    {
        std::print("Got disconnected while sending play again decision.\n");
        return std::make_tuple(wantsToPlayAgain, disconnected, oppDisconnected);
    }
    return std::make_tuple(wantsToPlayAgain, disconnected, oppDisconnected);
}
