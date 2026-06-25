#include "engine.hpp"
#include "interface.hpp"
#include "util.hpp"

#include <cstdint>
#include <chrono>
#include <thread>
#include <optional>
#include <string>
#include <functional>
#include <format>
#include <print>

std::optional<engine::Board> engine::humanTurn(Board board, std::string letter)
{
    while (true)
    {
        std::int8_t choice = interface::getHumanMove(board, letter);
        if (choice == -1)
        {
            return {};
        }
        std::optional<Board> newBoard = util::place(board, choice, letter);
        if (!newBoard)
        {
            std::string msg;
            if (letter == "X")
            {
                msg = interface::red(std::format("You can't play at {}!", choice));
            }
            else
            {
                msg = interface::cyan(std::format("You can't play at {}!", choice));
            }
            std::print("{}\n", msg);
        }
        else
        {
            return {newBoard};
        }
    }
}

engine::Board engine::cpuTurn(engine::Board board, std::string letter, std::function<std::uint8_t(Board)> strategy)
{
    std::function<std::string(std::string)> color;
    if (letter == "X")
    {
        color = interface::red;
    }
    else
    {
        color = interface::cyan;
    }
    std::string msg = color(std::format("CPU {} is taking its turn...", letter));
    std::print("{} ", msg);

    std::this_thread::sleep_for(std::chrono::milliseconds(750));

    std::uint8_t choice = strategy(board);
    msg = color(std::format("playing on {}\n", choice));
    std::print("{}\n", msg);

    return util::place(board, choice, letter).value();
}

bool engine::keepPlaying(engine::Board board)
{
    std::uint8_t winner = util::winner(board);

    if (winner == 'X')
    {
        std::string msg = interface::red(std::format("\n{} is the winner!\n", static_cast<char>(winner)));
        std::print("{}\n", msg);
        return false;
    }
    else if (winner == 'O')
    {
        std::string msg = interface::cyan(std::format("\n{} is the winner!\n", static_cast<char>(winner)));
        std::print("{}\n", msg);
        return false;
    }
    else if (util::full(board))
    {
        std::string msg = interface::green("\nStalemate.\n");
        std::print("{}\n", msg);
        return false;
    }
    else
    {
        return true;
    }
}

void engine::cpuVsCpu(std::function<std::uint8_t(Board)> strategyX, std::function<std::uint8_t(Board)> strategyO)
{
    Board board = util::makeBoard();
    while (true)
    {
        interface::show(board);
        board = engine::cpuTurn(board, "X", strategyX);
        if (!engine::keepPlaying(board))
        {
            break;
        }
        interface::show(board);
        board = engine::cpuTurn(board, "O", strategyO);
        if (!engine::keepPlaying(board))
        {
            break;
        }
    }
    interface::show(board);
}

void engine::cpuVsHuman(std::function<std::uint8_t(Board)> strategy)
{
    Board board = util::makeBoard();
    while (true)
    {
        interface::show(board);
        board = engine::cpuTurn(board, "X", strategy);
        if (!engine::keepPlaying(board))
        {
            break;
        }
        std::optional<Board> optBoard = engine::humanTurn(board, "O");
        if (!optBoard)
        {
            break;
        }
        board = optBoard.value();
        if (!engine::keepPlaying(board))
        {
            break;
        }
    }
    interface::show(board);
}

void engine::humanVsHuman()
{
    Board board = util::makeBoard();
    while (true)
    {
        std::optional<Board> optBoard = engine::humanTurn(board, "X");
        if (!optBoard)
        {
            break;
        }
        board = optBoard.value();
        if (!engine::keepPlaying(board))
        {
            break;
        }
        optBoard = engine::humanTurn(board, "O");
        if (!optBoard)
        {
            break;
        }
        board = optBoard.value();
        if (!engine::keepPlaying(board))
        {
            break;
        }
    }
    interface::show(board);
}

void engine::humanVsCpu(std::function<std::uint8_t(Board)> strategy)
{
    Board board = util::makeBoard();
    while (true)
    {
        std::optional<Board> optBoard = engine::humanTurn(board, "X");
        if (!optBoard)
        {
            break;
        }
        board = optBoard.value();
        if (!engine::keepPlaying(board))
        {
            break;
        }
        interface::show(board);
        board = engine::cpuTurn(board, "O", strategy);
        if (!engine::keepPlaying(board))
        {
            break;
        }
    }
    interface::show(board);
}
