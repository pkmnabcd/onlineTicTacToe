#pragma once

#include "GameState.hpp"
#include "Lobby.hpp"
#include "Player.hpp"

#include <array>
#include <cstdint>
#include <mutex>
#include <queue>
#include <tuple>

const std::size_t arraySize = static_cast<std::size_t>(UINT8_MAX) + 1;

// TODO: maybe make it more consistent that when invalidating/closing something
// you just need the right index.
namespace critical
{
    std::tuple<std::uint8_t, bool> getAvailableID(std::queue<std::uint8_t>& freeIDs, std::mutex& mut);
    void addIDToQueue(std::queue<std::uint8_t>& freeIDs, std::uint8_t id, std::mutex& mut);
    bool addPlayerToPlayers(std::array<Player, arraySize>& players, Player player, std::mutex& mut);
    void invalidatePlayer(std::array<Player, arraySize>& players, std::uint8_t playerID, std::mutex& mut);
    bool addLobbyToLobbies(std::array<Lobby, arraySize>& lobbies, Lobby lobby, std::mutex& mut);
    void closeLobby(std::array<Lobby, arraySize>& lobbies, Lobby lobby, std::mutex& mut);
    void closeLobbyIfOtherPlayerDisconnected(std::array<Lobby, arraySize>& lobbies, Lobby lobby, std::mutex& dataMut, std::mutex& disconnectMut);
    Player getGuestFromClientLobby(std::array<Lobby, arraySize>& lobbies, std::uint8_t client_id, std::mutex& mut);
    bool addGameStateToGameStates(std::array<GameState, arraySize>& gamestates, GameState gamestate, std::uint8_t hostID, std::mutex& mut);
    void invalidateGamestate(std::array<GameState, arraySize>& gamestates, std::uint8_t hostID, std::mutex& mut);
    void invalidateGamestateIfOtherPlayerDisconnected(std::array<GameState, arraySize>& gamestates, std::array<Lobby, arraySize>& lobbies, std::uint8_t hostID, std::mutex& dataMut, std::mutex& disconnectMut);
} // namespace critical
