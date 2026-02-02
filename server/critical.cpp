#include "critical.hpp"

#include "GameState.hpp"
#include "Lobby.hpp"
#include "Player.hpp"

#include <array>
#include <cstdint>
#include <queue>
#include <mutex>
#include <tuple>


std::tuple<std::uint8_t, bool> critical::getAvailableID(std::queue<std::uint8_t>& freeIDs, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    std::uint8_t client_id;
    bool noneAvailable = false;
    if (freeIDs.empty())
    {
        noneAvailable = true;
    }
    else
    {
        client_id = freeIDs.front();
        freeIDs.pop();
    }

    return std::make_tuple(client_id, noneAvailable);
}

void critical::addIDToQueue(std::queue<std::uint8_t>& freeIDs, std::uint8_t id, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    freeIDs.push(id);
}

bool critical::addPlayerToPlayers(std::array<Player, arraySize>& players, Player player, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    std::uint8_t playerID = player.m_id;
    if (players[playerID].m_isValidPlayer)
    {
        return false; // Shouldn't be editing player if there's already one there
    }
    else
    {
        players[playerID] = player;
        return true;
    }
}

void critical::invalidatePlayer(std::array<Player, arraySize>& players, std::uint8_t playerID, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    players[playerID].m_isValidPlayer = false;
}

bool critical::addLobbyToLobbies(std::array<Lobby, arraySize>& lobbies, Lobby lobby, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    std::uint8_t hostPlayerID = lobby.m_host.m_id;
    if (lobbies[hostPlayerID].m_isValid)
    {
        return false; // Shouldn't be editing lobby if there's already one there
    }
    else
    {
        lobbies[hostPlayerID] = lobby;
        return true;
    }
}

void critical::closeLobby(std::array<Lobby, arraySize>& lobbies, Lobby lobby, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    std::uint8_t hostPlayerID = lobby.m_host.m_id;
    lobbies[hostPlayerID].m_isValid = false;
}

void critical::closeLobbyIfOtherPlayerDisconnected(std::array<Lobby, arraySize>& lobbies, Lobby lobby, std::mutex& dataMut, std::mutex& disconnectMut)
{
    const std::lock_guard<std::mutex> lock(disconnectMut); // gets released when function returns
    std::uint8_t hostPlayerID = lobby.m_host.m_id;
    if (lobbies[hostPlayerID].m_someoneDisconnected)
    {
        critical::closeLobby(lobbies, lobby, dataMut);
    }
    else
    {
        lobbies[hostPlayerID].m_someoneDisconnected = true;
    }
}

Player critical::getGuestFromClientLobby(std::array<Lobby, arraySize>& lobbies, std::uint8_t client_id, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    return lobbies[client_id].m_guest;
}

bool critical::addGameStateToGameStates(std::array<GameState, arraySize>& gamestates, GameState gamestate, std::uint8_t hostID, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    if (gamestates[hostID].m_isValid)
    {
        return false; // Shouldn't be editing gamestate if there's already one there
    }
    else
    {
        gamestates[hostID] = gamestate;
        return true;
    }
}

void critical::invalidateGamestate(std::array<GameState, arraySize>& gamestates, std::uint8_t hostID, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    gamestates[hostID].m_isValid = false;
}

void critical::invalidateGamestateIfOtherPlayerDisconnected(std::array<GameState, arraySize>& gamestates, std::array<Lobby, arraySize>& lobbies, std::uint8_t hostID, std::mutex& dataMut, std::mutex& disconnectMut)
{
    const std::lock_guard<std::mutex> lock(disconnectMut); // gets released when function returns
    if (lobbies[hostID].m_someoneDisconnected)
    {
        critical::invalidateGamestate(gamestates, hostID, dataMut);
    }
    else
    {
        lobbies[hostID].m_someoneDisconnected = true;
    }
}
