#pragma once

#include <cstdint>
#include <queue>

namespace critical
{
    // TODO: update this to check if IDs list is empty
    // Maybe use structured bindings?
    std::uint8_t getAvailableID(std::queue<std::uint8_t>& freeIDs, bool* lock);
} // namespace critical
