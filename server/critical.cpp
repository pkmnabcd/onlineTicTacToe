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

auto critical::getAvailableID(std::queue<std::uint8_t>& freeIDs, bool* lock)
{
    while (testAndSet(lock))
    {
    }
    std::uint8_t client_id;
    bool success = true;
    if (freeIDs.empty())
    {
        success = false;
    }
    else
    {
        client_id = freeIDs.front();
        freeIDs.pop();
    }

    *lock = false;
    return std::make_tuple(client_id, success);
}
