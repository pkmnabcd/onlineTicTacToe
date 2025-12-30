#pragma once

#include <cstdint>
#include <queue>

namespace critical
{
    std::tuple<std::uint8_t, bool> getAvailableID(std::queue<std::uint8_t>& freeIDs, bool* lock);
} // namespace critical
