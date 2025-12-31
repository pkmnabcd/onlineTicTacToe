#include "critical.hpp"

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
