#include "ui.hpp"
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

std::string ui::black(std::string input)
{
    return std::format("\033[1;30m{}\033[0m", input);
}
std::string ui::red(std::string input)
{
    return std::format("\033[1;31m{}\033[0m", input);
}
std::string ui::green(std::string input)
{
    return std::format("\033[1;32m{}\033[0m", input);
}
std::string ui::yellow(std::string input)
{
    return std::format("\033[1;33m{}\033[0m", input);
}
std::string ui::blue(std::string input)
{
    return std::format("\033[1;34m{}\033[0m", input);
}
std::string ui::magenta(std::string input)
{
    return std::format("\033[1;35m{}\033[0m", input);
}
std::string ui::cyan(std::string input)
{
    return std::format("\033[1;36m{}\033[0m", input);
}
std::string ui::white(std::string input)
{
    return std::format("\033[1;37m{}\033[0m", input);
}

std::string color(std::string s)
{
    if (s == "X")
    {
        return ui::red(s);
    }
    else if (s == "O")
    {
        return ui::cyan(s);
    }
    else
    {
        return ui::white(s);
    }
}

bool boardEmpty(ui::Board board)
{
    return board[0].empty() && board[1].empty() && board[2].empty();
}

void ui::logo()
{
    std::print("\n");
    std::print("{} {} {}\n", ui::red("888888888               "), ui::white("888888888                "), ui::cyan("888888888                 "));
    std::print("{} {} {}\n", ui::red("\"8888888\" ooooo  ooooo  "), ui::white("\"8888888\" ooo     ooooo  "), ui::cyan("\"8888888\"  ooo    ooooo   "));
    std::print("{} {} {}\n", ui::red("   888     888  88   8  "), ui::white("   888    888    88   8  "), ui::cyan("   888   88   88  8       "));
    std::print("{} {} {}\n", ui::red("   888     888  88      "), ui::white("   888   8ooo8   88      "), ui::cyan("   888   88   88  8ooooo  "));
    std::print("{} {} {}\n", ui::red("   888     888  88    88"), ui::white("   888  888  888 88    88"), ui::cyan("   888   88o  88 o88      "));
    std::print("{} {} {}\n", ui::red("   888    88888  888888\""), ui::white("   888  888  888  888888\""), ui::cyan("   888   \"888888 888888888"));
    std::print("                                                            by {}(tm)\n", ui::yellow("DuckieCorp"));
    std::print("{}\n", ui::green("\nWOULD YOU LIKE TO PLAY A GAME?\n"));
}

void ui::show(ui::Board board)
{
    if (!boardEmpty(board))
    {
        std::print(" {} | {} | {}\n---+---+---\n {} | {} | {}\n---+---+---\n {} | {} | {}\n\n",
                   color(board[0][0]), color(board[0][1]), color(board[0][2]),
                   color(board[1][0]), color(board[1][1]), color(board[1][2]),
                   color(board[2][0]), color(board[2][1]), color(board[2][2]));

    }
}

std::int8_t ui::getHumanMove(ui::Board board, std::string letter)
{
    std::array<std::string, 9> straightBoard = util::get1dFrom2dBoard(board);
    while (true)
    {
        ui::show(board);

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
                    std::print("I don't understand '{}', try again! (Input valid char)\n", static_cast<char>(choice));
                }
            }
            else
            {
                choice = util::toIntVal(choice);
                bool isValidMove = choice != 0 && util::isDigit(straightBoard[choice - 1].at(0));
                if (isValidMove)
                {
                    return choice;
                }
                std::print("You must choose an open space.\n");
            }
        }
    }
}

std::uint8_t ui::player_select()
{
    while (true)
    {
        std::print("0) {} {} vs. {} {}\n", ui::red("X"), ui::green("CPU  "), ui::cyan("O"), ui::green("CPU"));
        std::print("1) {} {} vs. {} {}\n", ui::red("X"), ui::white("Human"), ui::cyan("O"), ui::green("CPU"));
        std::print("2) {} {} vs. {} {}\n", ui::red("X"), ui::green("CPU  "), ui::cyan("O"), ui::white("Human"));
        std::print("3) {} {} vs. {} {}\n", ui::red("X"), ui::white("Human"), ui::cyan("O"), ui::white("Human"));
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

std::uint8_t ui::selectGameMode()
{
    while (true)
    {
        std::print("0) {}\n1) {}\n", ui::green("Local Play"), ui::magenta("Online Play"));
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

void ui::home()
{
    rlutil::locate(1, 1);
}

void ui::clear()
{
    rlutil::cls();
}

std::string ui::getUsername()
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

bool ui::selectHostLobby()
{
    while (true)
    {
        std::uint8_t choice;
        std::string input;
        std::print("\n0) {}\n1) {}\n", ui::green("Host"), ui::magenta("Join"));
        std::print("Do you want to host a lobby or join an existing lobby?> ");
        std::cin >> input;
        std::print("\n");

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

std::tuple<std::uint8_t, std::string> ui::chooseLobby(std::vector<std::tuple<std::string, std::uint8_t>> lobbies)
{
    std::uint8_t id = 0;
    std::string oppName = "";

    std::print("\nOpen lobbies:\nID: name\n");
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
        catch (std::invalid_argument const&)
        {
            std::print("Please enter a valid ID.\n");
            continue;
        }
        catch (std::out_of_range const&)
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
                oppName = std::get<0>(lobby);
                notValidChoice = false;
                break;
            }
        }
        if (notValidChoice)
        {
            std::print("Please enter a valid ID.\n");
        }
    }
    return std::make_tuple(id, oppName);
}

// TODO: with these interactive functions, make sure you can quit anytime
std::tuple<bool, bool> ui::chooseRedOrBlue()
{
    bool choosingRed = false;
    bool quitting = false;

    std::print("\n0) {}\n1) {}\n", ui::red("Red Player (X)"), ui::cyan("Blue Player (O)"));
    bool notValidChoice = true;
    while (notValidChoice)
    {
        std::print("Pick whether you want to go {} or {}> ", ui::red("first"), ui::cyan("second"));
        std::string input;
        std::cin >> input;
        std::print("\n");
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
                notValidChoice = false;
                quitting = true;
            }
            else
            {
                std::print("\nInvalid selection!\n\n");
            }
        }
    }
    return std::make_tuple(choosingRed, quitting);
}

void ui::printOppTurnMessage(std::uint8_t movePos, bool isRed, std::string name)
{
    std::string msg;
    if (isRed)
    {
        msg = ui::red(std::format("{} (X) took their turn... played on {}\n", name, movePos));
    }
    else
    {
        msg = ui::cyan(std::format("{} (O) took their turn... played on {}\n", name, movePos));
    }
    std::print("{}\n", msg);
}

void ui::printWinnerMessage(util::Winner winner)
{
    std::string msg;
    if (winner == util::Winner::Red)
    {
        msg = ui::red("\nX is the winner!\n");
    }
    else if (winner == util::Winner::Blue)
    {
        msg = ui::cyan("\nO is the winner!\n");
    }
    else
    {
        msg = ui::green("\nStalemate.\n");
    }
    std::print("{}\n", msg);
}

bool ui::playAgain(std::string oppName)
{
    std::print("0) Yes\n1) No\n");
    bool notValidChoice = true;
    bool playAgain = false;
    while (notValidChoice)
    {
        std::print("Play again with {}? > ", oppName);
        std::string input;
        std::cin >> input;
        std::print("\n");
        if (input.size() != 1)
        {
            std::print("\nInvalid selection! (Input one char)\n\n");
        }
        else
        {
            if (input.at(0) == '0')
            {
                playAgain = true;
                notValidChoice = false;
            }
            else if (input.at(0) == '1')
            {
                notValidChoice = false;
            }
            else
            {
                std::print("\nInvalid selection!\n\n");
            }
        }
    }
    return playAgain;
}
