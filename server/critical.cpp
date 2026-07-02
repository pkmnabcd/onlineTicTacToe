#include "critical.hpp"

#include "GameState.hpp"
#include "Lobby.hpp"
#include "Player.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <queue>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>


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
    if (players[playerID].m_isValid)
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
    players[playerID].m_isValid = false;
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

void critical::invalidateLobby(std::array<Lobby, arraySize>& lobbies, std::uint8_t hostID, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    lobbies[hostID].m_isValid = false;
}

void critical::invalidateLobbyIfOtherPlayerDisconnected(std::array<Lobby, arraySize>& lobbies, std::uint8_t hostID, std::mutex& dataMut, std::mutex& disconnectMut)
{
    const std::lock_guard<std::mutex> lock(disconnectMut); // gets released when function returns
    // check if guest disconnected or if there was never a guest
    if (lobbies[hostID].m_someoneDisconnected || !lobbies[hostID].m_guest.m_isValid)
    {
        critical::invalidateLobby(lobbies, hostID, dataMut);
    }
    else
    {
        lobbies[hostID].m_someoneDisconnected = true;
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

void critical::invalidateGamestateIfOtherPlayerDisconnected(std::array<GameState, arraySize>& gamestates, std::uint8_t hostID, std::mutex& dataMut, std::mutex& disconnectMut)
{
    const std::lock_guard<std::mutex> lock(disconnectMut); // gets released when function returns
    if (gamestates[hostID].m_someoneDisconnected)
    {
        critical::invalidateGamestate(gamestates, hostID, dataMut);
    }
    else
    {
        gamestates[hostID].m_someoneDisconnected = true;
    }
}

void critical::invalidatePlayerOnceLobbyIsInvalid(std::array<Player, arraySize>& players, std::array<Lobby, arraySize>& lobbies, std::uint8_t playerID, std::mutex& mut)
{
    mut.lock();
    while (lobbies[playerID].m_isValid) // wait until lobby is invalidated
    {
            mut.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            mut.lock();
    }
    players[playerID].m_isValid = false;
    mut.unlock();
}

std::vector<Lobby> critical::getOpenLobbies(std::array<Lobby, arraySize>& lobbies, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    auto openLobbies = std::vector<Lobby>();
    for (Lobby& lobby : lobbies)
    {
        if (lobby.m_isValid && !lobby.m_guest.m_isValid)
        {
            openLobbies.push_back(lobby);
        }
    }
    return openLobbies;
}

bool critical::addGuestToLobby(std::array<Lobby, arraySize>& lobbies, std::uint8_t hostID, Player guest, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    // TODO: somehow address the risk of a guest selecting a lobby after the
    // original host leaves and another takes their place.
    bool canAddGuest = !lobbies[hostID].m_guest.m_isValid && lobbies[hostID].m_isValid && !lobbies[hostID].m_someoneDisconnected;
    if (canAddGuest)
    {
        lobbies[hostID].m_guest = guest;
    }
    return canAddGuest;
}

std::tuple<bool, bool> critical::hostPickedRed(std::array<GameState, arraySize>& gamestates, std::array<Lobby, arraySize>& lobbies, std::uint8_t hostID, std::mutex& mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    bool hostPickedRed = false;
    bool disconnected = false;
    if (lobbies[hostID].m_someoneDisconnected)
    {
        disconnected = true;
    }
    else
    {
        hostPickedRed = gamestates[hostID].m_redPlayer.m_id == hostID;
    }
    return std::make_tuple(hostPickedRed, disconnected);
}
