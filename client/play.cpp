#include "play.hpp"

#include "interface.hpp"
#include "matchmaking.hpp"
#include "util.hpp"

#include <cstdint>
#include <format>
#include <print>
#include <tuple>


void showBlueRedsFirstTurn(Board board)
{
}

std::uint8_t getMovePosition(Board prevBoard, Board currBoard)
{
    /*
     * Returns 0 for no difference, 1-9 for the position of a difference, and 10 for multiple differences (bad)
    */

    std::array<std::string, 9> prevBoardStraight = util::get1dFrom2dBoard(prevBoard);
    std::array<std::string, 9> currBoardStraight = util::get1dFrom2dBoard(currBoard);

    std::uint8_t pos = 0;
    for (std::uint8_t i = 0; i < 9; i++)
    {
        if (prevBoard[i].at(0) != currBoard[i].at(0))
        {
            if (pos != 0)
            {
                pos = 10; // should only be one difference
            }
            else
            {
                pos = i+1;
            }
        }
    }
    return pos;
}

void printOppTurnMessage(std::uint8_t movePos, bool isRed, std::string name)
{
    std::string msg;
    if (isRed)
    {
        msg = interface::red(std::format("{} X took their turn... played on {}", name, movePos));
    }
    else
    {
        msg = interface::cyan(std::format("{} O took their turn... played on {}", name, movePos));
    }
    std::print("{}\n", msg);
}

void printWinnerMessage(matchmaking::Winner winner)
{
    std::string msg;
    if (winner == matchmaking::Winner::Red)
    {
        msg = interface::red("\nX is the winner!\n");
    }
    else if (winner == matchmaking::Winner::Blue)
    {
        msg = interface::cyan("\nO is the winner!\n");
    }
    else
    {
        msg = interface::green("\nStalemate.\n");
    }
    std::print("{}\n", msg);
}

std::tuple<bool, bool, bool> play::playGame(int serv_fd, bool isRed)
{
    /*
     * Returns [wantsToPlayAgain: bool, disconnected: bool, oppDisconnected: bool]
     */
    bool wantsToPlayAgain = false;
    bool disconnected = false;
    bool oppDisconnected = false;

    bool firstTurn = true;
    while (true) // game loop
    {
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
        auto [previousBoard, disconnectedTmp1] = matchmaking::getBoardState(serv_fd);
        disconnected = disconnectedTmp1;
        if (disconnected)
        {
            std::print("Got disconnected while waiting for board update.\n");
            return std::make_tuple(wantsToPlayAgain, disconnected, oppDisconnected);
        }
        std::print("Got the board state!\n");

        // If the game is over, show the final move and break out of game loop
        if (!continuePlay)
        {
            // TODO: add the message of the winner here
            break; // Go to code that asks whether to play again.
        }

        // TODO: put function where blue is shown red's first turn here
        // since that won't happen automatically

        // Get move from user and send to server
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
