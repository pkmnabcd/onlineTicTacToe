#include "critical.hpp"

#include <array>
#include <cstdint>
#include <queue>
#include <tuple>

bool testAndSet(bool* target)
{
    bool returnVal = *target;
    *target = true;
    return returnVal;
}

std::tuple<std::uint8_t, bool> critical::getAvailableID(std::queue<std::uint8_t>& freeIDs, bool* lock)
{
    while (testAndSet(lock))
    {
    }
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

    *lock = false;
    return std::make_tuple(client_id, noneAvailable);
}

void critical::addIDToQueue(std::queue<std::uint8_t>& freeIDs, std::uint8_t id, bool* lock)
{
    while (testAndSet(lock))
    {
    }
    freeIDs.push(id);
    *lock = false;
}

bool critical::addPlayerToPlayers(std::array<Player, arraySize>& players, Player player, bool* lock)
{
    while (testAndSet(lock))
    {
    }
    std::uint8_t playerID = player.m_id;
    if (player.m_isValidPlayer)
    {
        *lock = false;
        return false; // Shouldn't be editing player if there's already one there
    }
    else
    {
        players[playerID] = player;
        *lock = false;
        return true;
    }
}

void critical::invalidatePlayer(std::array<Player, arraySize>& players, std::uint8_t playerID, bool* lock)
{
    while (testAndSet(lock))
    {
    }
    players[playerID].m_isValidPlayer = false;
    *lock = false;
}
