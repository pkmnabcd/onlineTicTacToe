#pragma once

#include "util.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

namespace interface
{
    using Board = std::array<std::array<std::string, 3>, 3>;

    void logo();
    void show(Board board);
    std::int8_t getHumanMove(Board board, std::string letter);
    std::uint8_t player_select();
    std::uint8_t selectGameMode();
    std::string getUsername();

    std::string black(std::string input);
    std::string red(std::string input);
    std::string green(std::string input);
    std::string yellow(std::string input);
    std::string blue(std::string input);
    std::string magenta(std::string input);
    std::string cyan(std::string input);
    std::string white(std::string input);

    void home();
    void clear();

    // Multiplayer specific functions
    bool selectHostLobby();
    std::tuple<std::uint8_t, std::string> chooseLobby(std::vector<std::tuple<std::string, std::uint8_t>> lobbies);
    bool chooseRedOrBlue();
    void printOppTurnMessage(std::uint8_t movePos, bool isRed, std::string name);
    void printWinnerMessage(util::Winner winner);
    bool playAgain(std::string oppName);
} // namespace interface
