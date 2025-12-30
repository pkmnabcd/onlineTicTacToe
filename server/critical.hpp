#pragma once

#include <cstdint>
#include <queue>

namespace critical
{
    auto getAvailableID(std::queue<std::uint8_t>& freeIDs, bool* lock);
} // namespace critical
