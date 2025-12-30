#include "GameState.hpp"
#include "Player.hpp"
#include "critical.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"

#include <array>
#include <print>
#include <queue>
#include <tuple>
#include <unistd.h>

void initializeFreeIDs(std::queue<std::uint8_t>& freeIDsQueue, std::size_t IDCount)
{
    for (std::size_t i = 0; i < IDCount; i++)
    {
        freeIDsQueue.push(i);
    }
}

void cleanupChildProcess(int client_fd)
{
    networking::closeFd(client_fd);
    exit(0); // Kill child process
}

int main()
{

    int serv_fd = networking::initServer();

    const std::size_t arraySize = static_cast<std::size_t>(UINT8_MAX) + 1;
    std::array<Player, arraySize> players;
    std::array<GameState, arraySize> gameStates;

    std::queue<std::uint8_t> freeIDs = std::queue<std::uint8_t>();
    initializeFreeIDs(freeIDs, arraySize);
    bool lock = false; // Make sure lock is unlocked
    bool* lockPtr = &lock;

    while (true)
    {
        int client_fd = networking::acceptConnection(serv_fd);
        if (client_fd == -1)
        {
            continue;
        }

        if (!fork()) // This is the child process associated with this player
        {
            networking::closeFd(serv_fd); // Child does not need this, will stay open in main process.

            bool client_disconnected = false;

            auto [client_id, noneAvailable] = critical::getAvailableID(freeIDs, lockPtr);
            if (noneAvailable)
            {
                cleanupChildProcess(client_id); // TODO: Make sure that client knows why they got booted
            }
            auto [client_player, disconnectedTmp0, isHosting] = matchmaking::getClientInfo(client_fd, client_id);
            client_disconnected = disconnectedTmp0;

            if (client_disconnected)
            {
                cleanupChildProcess(client_id);
            }

            // TODO: Get from client info (first char?) whether the client is hosting a match or
            // if they're joinging someone else. Also assign ID and add to player list

            // TODO: If Hosting, create a 'lobby' in which they can play games.
            // Lobby can only have the two players, but do this so games can be repeated and
            // players can swap being the red and blue player

            // TODO: If joining, give a list of lobbies that need a second player.

            cleanupChildProcess(client_id);
        }
        networking::closeFd(client_fd); // Parent doesn't use this anymore, will stay open for child.
    }

    networking::closeFd(serv_fd);

    return 0;
}
