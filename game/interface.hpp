#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace interface
{
    using Board = std::array<std::array<std::string, 3>, 3>;

    void logo();
    void show(Board board);
    std::int8_t getHumanMove(Board board, std::string letter);
    std::uint8_t player_select();
    std::uint8_t selectGameMode();

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
} // namespace interface
