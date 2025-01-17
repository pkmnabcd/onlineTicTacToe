#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace engine
{
    using Board = std::array<std::array<std::string, 3>, 3>;

    std::optional<Board> humanTurn(Board board, std::string letter);
    Board cpuTurn(Board board, std::string letter, std::function<std::uint8_t(Board)> strategy);

    bool keepPlaying(Board board);
} // namespace engine
