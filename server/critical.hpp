#pragma once

#include "GameState.hpp"
#include "Lobby.hpp"
#include "Player.hpp"
#include "settings.hpp"

#include <array>
#include <cstdint>
#include <mutex>
#include <queue>
#include <tuple>
#include <vector>

namespace critical
{
    std::tuple<std::uint8_t, bool> getAvailableID(std::queue<std::uint8_t>& freeIDs, std::mutex& mut);
    void addIDToQueue(std::queue<std::uint8_t>& freeIDs, std::uint8_t id, std::mutex& mut);
    bool addPlayerToPlayers(std::array<Player, arraySize>& players, Player player, std::mutex& mut);
    void invalidatePlayer(std::array<Player, arraySize>& players, std::uint8_t playerID, std::mutex& mut);
    bool addLobbyToLobbies(std::array<Lobby, arraySize>& lobbies, Lobby lobby, std::mutex& mut);
    void invalidateLobby(std::array<Lobby, arraySize>& lobbies, std::uint8_t hostID, std::mutex& mut);
    void invalidateLobbyIfOtherPlayerDisconnected(std::array<Lobby, arraySize>& lobbies, std::uint8_t hostID, std::mutex& dataMut, std::mutex& disconnectMut);
    Player getGuestFromClientLobby(std::array<Lobby, arraySize>& lobbies, std::uint8_t client_id, std::mutex& mut);
    bool addGameStateToGameStates(std::array<GameState, arraySize>& gamestates, GameState gamestate, std::uint8_t hostID, std::mutex& mut);
    void invalidateGamestate(std::array<GameState, arraySize>& gamestates, std::uint8_t hostID, std::mutex& mut);
    void invalidateGamestateIfOtherPlayerDisconnected(std::array<GameState, arraySize>& gamestates, std::uint8_t hostID, std::mutex& dataMut, std::mutex& disconnectMut);
    void invalidatePlayerOnceLobbyIsInvalid(std::array<Player, arraySize>& players, std::array<Lobby, arraySize>& lobbies, std::uint8_t playerID, std::mutex& mut);
    std::vector<Lobby> getOpenLobbies(std::array<Lobby, arraySize>& lobbies, std::mutex& mut);
} // namespace critical
