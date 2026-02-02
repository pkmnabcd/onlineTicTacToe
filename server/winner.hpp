#pragma once

#include <array>
#include <cstdint>
#include <string>

using StraightBoard = std::array<std::string, 9>;

namespace winner
{
    std::uint8_t winner(StraightBoard board);
} // namespace winner
