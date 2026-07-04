#pragma once

#include <array>
#include <cstdint>

namespace ai
{
    using Board = std::array<std::array<char, 3>, 3>;
    std::uint8_t strategyDumb(Board board);
    std::uint8_t strategyRandom(ai::Board board);
    std::uint8_t strategyOracle(Board board);
} // namespace ai
