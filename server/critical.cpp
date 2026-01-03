#include "critical.hpp"

#include <array>
#include <cstdint>
#include <queue>
#include <mutex>
#include <tuple>


std::tuple<std::uint8_t, bool> critical::getAvailableID(std::queue<std::uint8_t>& freeIDs, std::mutex mut)
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

void critical::addIDToQueue(std::queue<std::uint8_t>& freeIDs, std::uint8_t id, std::mutex mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    freeIDs.push(id);
}

bool critical::addPlayerToPlayers(std::array<Player, arraySize>& players, Player player, std::mutex mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    std::uint8_t playerID = player.m_id;
    if (player.m_isValidPlayer)
    {
        return false; // Shouldn't be editing player if there's already one there
    }
    else
    {
        players[playerID] = player;
        return true;
    }
}

void critical::invalidatePlayer(std::array<Player, arraySize>& players, std::uint8_t playerID, std::mutex mut)
{
    const std::lock_guard<std::mutex> lock(mut); // gets released when function returns
    players[playerID].m_isValidPlayer = false;
}
