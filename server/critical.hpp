#pragma once

#include "Player.hpp"

#include <array>
#include <cstdint>
#include <queue>

const std::size_t arraySize = static_cast<std::size_t>(UINT8_MAX) + 1;

namespace critical
{
    std::tuple<std::uint8_t, bool> getAvailableID(std::queue<std::uint8_t>& freeIDs, bool* lock);
    void addIDToQueue(std::queue<std::uint8_t>& freeIDs, std::uint8_t id, bool* lock);
    bool addPlayerToPlayers(std::array<Player, arraySize>& players, Player player, bool* lock);
    void invalidatePlayer(std::array<Player, arraySize>& players, std::uint8_t playerID, bool* lock);
} // namespace critical
