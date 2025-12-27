#include "critical.hpp"

#include <cstdint>
#include <queue>

bool testAndSet(bool* target)
{
    bool returnVal = *target;
    *target = true;
    return returnVal;
}

std::uint8_t critical::getAvailableID(std::queue<std::uint8_t>& freeIDs, bool* lock)
{
    std::uint8_t client_id;
    do
    {
        while (testAndSet(lock))
        {
        }
        client_id = freeIDs.front();
        freeIDs.pop();
        *lock = false;
    } while (true);
    return client_id;
}
