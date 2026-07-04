#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace engine
{
    using Board = std::array<std::array<char, 3>, 3>;

    std::optional<Board> humanTurn(Board board, char letter);
    Board cpuTurn(Board board, char letter, std::function<std::uint8_t(Board)> strategy);

    bool keepPlaying(Board board);
    void cpuVsCpu(std::function<std::uint8_t(Board)> strategyX, std::function<std::uint8_t(Board)> strategyO);
    void cpuVsHuman(std::function<std::uint8_t(Board)> strategy);
    void humanVsHuman();
    void humanVsCpu(std::function<std::uint8_t(Board)> strategy);
} // namespace engine
