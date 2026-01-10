#pragma once

#include "Lobby.hpp"
#include "Player.hpp"

#include <array>
#include <cstdint>
#include <mutex>
#include <queue>

const std::size_t arraySize = static_cast<std::size_t>(UINT8_MAX) + 1;

namespace critical
{
    std::tuple<std::uint8_t, bool> getAvailableID(std::queue<std::uint8_t>& freeIDs, std::mutex& mut);
    void addIDToQueue(std::queue<std::uint8_t>& freeIDs, std::uint8_t id, std::mutex& mut);
    bool addPlayerToPlayers(std::array<Player, arraySize>& players, Player player, std::mutex& mut);
    void invalidatePlayer(std::array<Player, arraySize>& players, std::uint8_t playerID, std::mutex& mut);
    bool addLobbyToLobbies(std::array<Lobby, arraySize>& lobbies, Lobby lobby, std::mutex& mut);
    Player getGuestFromClientLobby(std::array<Lobby, arraySize>& lobbies, std::uint8_t client_id, std::mutex& mut);
} // namespace critical
