#include "interface.hpp"
#include "util.hpp"
#include "rlutil.h"

#include <array>
#include <cstdint>
#include <format>
#include <iostream>
#include <print>
#include <string>

std::string interface::black(std::string input)
{
    return std::format("\033[1;30m{}\033[0m", input);
}
std::string interface::red(std::string input)
{
    return std::format("\033[1;31m{}\033[0m", input);
}
std::string interface::green(std::string input)
{
    return std::format("\033[1;32m{}\033[0m", input);
}
std::string interface::yellow(std::string input)
{
    return std::format("\033[1;33m{}\033[0m", input);
}
std::string interface::blue(std::string input)
{
    return std::format("\033[1;34m{}\033[0m", input);
}
std::string interface::magenta(std::string input)
{
    return std::format("\033[1;35m{}\033[0m", input);
}
std::string interface::cyan(std::string input)
{
    return std::format("\033[1;36m{}\033[0m", input);
}
std::string interface::white(std::string input)
{
    return std::format("\033[1;37m{}\033[0m", input);
}

std::string color(std::string s)
{
    if (s == "X")
    {
        return interface::red(s);
    }
    else if (s == "O")
    {
        return interface::cyan(s);
    }
    else
    {
        return interface::white(s);
    }
}

bool boardEmpty(interface::Board board)
{
    return board[0].empty() && board[1].empty() && board[2].empty();
}

void interface::logo()
{
    std::print("\n");
    std::print("{} {} {}\n", interface::red("888888888               "), interface::white("888888888                "), interface::cyan("888888888                 "));
    std::print("{} {} {}\n", interface::red("\"8888888\" ooooo  ooooo  "), interface::white("\"8888888\" ooo     ooooo  "), interface::cyan("\"8888888\"  ooo    ooooo   "));
    std::print("{} {} {}\n", interface::red("   888     888  88   8  "), interface::white("   888    888    88   8  "), interface::cyan("   888   88   88  8       "));
    std::print("{} {} {}\n", interface::red("   888     888  88      "), interface::white("   888   8ooo8   88      "), interface::cyan("   888   88   88  8ooooo  "));
    std::print("{} {} {}\n", interface::red("   888     888  88    88"), interface::white("   888  888  888 88    88"), interface::cyan("   888   88o  88 o88      "));
    std::print("{} {} {}\n", interface::red("   888    88888  888888\""), interface::white("   888  888  888  888888\""), interface::cyan("   888   \"888888 888888888"));
    std::print("                                                            by {}(tm)\n", interface::yellow("DuckieCorp"));
    std::print("{}\n", interface::green("\nWOULD YOU LIKE TO PLAY A GAME?\n"));
}

void interface::show(interface::Board board)
{
    if (!boardEmpty(board))
    {
        std::print(" {} | {} | {}\n---+---+---\n {} | {} | {}\n---+---+---\n {} | {} | {}\n\n",
                   color(board[0][0]), color(board[0][1]), color(board[0][2]),
                   color(board[1][0]), color(board[1][1]), color(board[1][2]),
                   color(board[2][0]), color(board[2][1]), color(board[2][2]));

    }
}

std::int8_t interface::getHumanMove(interface::Board board, std::string letter)
{
    while (true)
    {
        interface::show(board);

        std::string input;
        std::uint8_t choice;
        std::print("Place your '{}' (or 'Q' to quit)> ", color(letter));
        std::cin >> input;

        if (input.size() != 1)
        {
            std::print("I don't understand '{}', try again! (Input one char)\n", input);
        }
        else
        {
            choice = input[0];
            if (!util::isDigit(choice))
            {
                if (util::toLower(choice) == 'q')
                {
                    return -1;  // Sign to quit
                }
                else 
                {
                    std::print("I don't understand '{}', try again! (Input valid char)\n", choice);
                }
            }
            else
            {
                return util::toIntVal(choice);
            }
        }
    }
}

std::uint8_t interface::player_select()
{
    while (true)
    {
        std::print("0) {} {} vs. {} {}\n", interface::red("X"), interface::green("CPU  "), interface::cyan("O"), interface::green("CPU"));
        std::print("1) {} {} vs. {} {}\n", interface::red("X"), interface::white("Human"), interface::cyan("O"), interface::green("CPU"));
        std::print("2) {} {} vs. {} {}\n", interface::red("X"), interface::green("CPU  "), interface::cyan("O"), interface::white("Human"));
        std::print("3) {} {} vs. {} {}\n", interface::red("X"), interface::white("Human"), interface::cyan("O"), interface::white("Human"));
        std::print("Choose game mode [0-3] or Q to quit > ");

        std::string input;
        std::cin >> input;
        if (input.size() != 1)
        {
            std::print("\nInvalid selection! (Input one char)\n\n");
        }
        else
        {
            std::uint8_t choice = input[0];
            if (choice == '0' || choice == '1' || choice == '2' || choice == '3')
            {
                return util::toIntVal(choice);
            }
            else if (util::toLower(choice) == 'q')
            {
                return 'q';
            }
            else
            {
                std::print("\nInvalid selection!\n\n");
            }
        }
    }

    return 0;  // Temp
}

std::uint8_t interface::selectGameMode()
{
    while (true)
    {
        std::print("0) {}\n1) {}\n", interface::green("Local Play"), interface::magenta("Online Play"));
        std::print("Choose game mode [0/1] or Q to quit > ");

        std::string input;
        std::cin >> input;
        if (input.size() != 1)
        {
            std::print("\nInvalid selection! (Input one char)\n\n");
        }
        else
        {
            std::uint8_t choice = input[0];
            if (choice == '0' || choice == '1')
            {
                return util::toIntVal(choice);
            }
            else if (util::toLower(choice) == 'q')
            {
                return 'q';
            }
            else
            {
                std::print("\nInvalid selection!\n\n");
            }
        }
    }
    return 0;
}

void interface::home()
{
    rlutil::locate(1, 1);
}

void interface::clear()
{
    rlutil::cls();
}
