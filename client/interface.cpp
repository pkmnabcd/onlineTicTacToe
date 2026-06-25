#include "interface.hpp"
#include "util.hpp"
#include "rlutil.h"

#include <array>
#include <cstdint>
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <tuple>
#include <vector>

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
        std::print("Choose game mode [0/1] or Q to quit> ");

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

std::string interface::getUsername()
{
    while (true)
    {
        std::string input;
        std::print("Pick a username (max 14 characters) for this online session> ");
        std::cin >> input;

        if (input.size() > 14)
        {
            std::print("Invalid username. Must be only 14 ASCII letters, numbers or symbols with no spaces/tabs.\n");
        }
        else if (input.size() == 0)
        {
            std::print("Invalid username. Username can't be blank.\n");
        }
        else
        {
            bool invalid = false;
            for (std::uint8_t i = 0; i < input.size(); i++)
            {
                char c = input.at(i);
                if (c < 33 || c > 126)
                {
                    invalid = true;
                }
            }
            if (invalid)
            {
                std::print("Invalid username. Must be only 14 ASCII letters, numbers or symbols (less than ASCII code 127) with no spaces/tabs.\n");
                continue;
            }
            else
            {
                return input;
            }
        }
    }
}

bool interface::selectHostLobby()
{
    while (true)
    {
        std::uint8_t choice;
        std::string input;
        std::print("0) {}\n1) {}\n", interface::green("Host"), interface::magenta("Join"));
        std::print("Do you want to host a lobby or join an existing lobby?> ");
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
                std::print("I don't understand '{}', try again! (Input valid char)\n", choice);
            }
            else
            {
                std::uint8_t val = util::toIntVal(choice);
                if (val == 0)
                {
                    return true;
                }
                else if (val == 1)
                {
                    return false;
                }
                else
                {
                    std::print("Invalid number. Should be 0 or 1.\n");
                }
            }
        }
    }
}

std::uint8_t interface::chooseLobby(std::vector<std::tuple<std::string, std::uint8_t>> lobbies)
{
    std::uint8_t id = 0;
    for (auto& lobby : lobbies)
    {
        std::string hostName = std::get<0>(lobby);
        std::uint8_t hostID = std::get<1>(lobby);
        std::print("{}: {}\n", hostID, hostName);
    }

    bool notValidChoice = true;
    while (notValidChoice)
    {
        std::print("Enter the ID number of the lobby you want to join> ");
        std::string input;
        std::cin >> input;
        if (input.size() < 1 || input.size() > 3)
        {
            std::print("Enter a 1-3 digit number (no negatives)\n");
            continue;
        }
        // Get the id from the string
        try
        {
            const unsigned long int idInt = std::stoul(input);
            if (idInt > UINT8_MAX)
            {
                std::print("Please enter a valid ID.\n");
                continue;
            }
            id = static_cast<std::uint8_t>(idInt);
        }
        catch (std::invalid_argument const& ex)
        {
            std::print("Please enter a valid ID.\n");
            continue;
        }
        catch (std::out_of_range const& ex)
        {
            std::print("Please enter a valid ID.\n");
            continue;
        }

        // Check if the id is in the list of open lobbies
        for (auto& lobby : lobbies)
        {
            std::uint8_t hostID = std::get<1>(lobby);
            if (hostID == id)
            {
                notValidChoice = false;
                break;
            }
        }
        if (notValidChoice)
        {
            std::print("Please enter a valid ID.\n");
        }
    }
    return id;
}

// TODO: with these interactive functions, make sure you can quit anytime
bool interface::chooseRedOrBlue()
{
    bool choosingRed = false;

    std::print("0) {}\n1) {}\n", interface::red("Red Player (X)"), interface::cyan("Blue Player (O)"));
    bool notValidChoice = true;
    while (notValidChoice)
    {
        std::print("Pick whether you want to go {} or {}> ", interface::red("first"), interface::cyan("second"));
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
                choosingRed = choice == '0';
                notValidChoice = false;
            }
            else if (util::toLower(choice) == 'q')
            {
                // TODO: actually finish this
                notValidChoice = true;
            }
            else
            {
                std::print("\nInvalid selection!\n\n");
            }
        }
    }
    return choosingRed;
}
