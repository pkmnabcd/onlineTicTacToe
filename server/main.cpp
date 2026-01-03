#include "GameState.hpp"
#include "Player.hpp"
#include "critical.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"

#include <array>
#include <mutex>
#include <print>
#include <queue>
#include <thread>
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

void manageClient(int client_fd, std::array<Player, arraySize>& players, std::array<GameState, arraySize>& gameStates, std::queue<std::uint8_t> freeIDs, std::mutex dataMutex)
{
    bool client_disconnected = false;

    auto [client_id, noneAvailable] = critical::getAvailableID(freeIDs, dataMutex);
    if (noneAvailable)
    {
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        cleanupChildProcess(client_fd); // TODO: Make sure that client knows why they got booted
    }
    auto [client_player, disconnectedTmp0, isHosting] = matchmaking::getClientInfo(client_fd, client_id);
    client_disconnected = disconnectedTmp0;

    if (client_disconnected)
    {
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        cleanupChildProcess(client_fd);
    }
    bool playerAdded = critical::addPlayerToPlayers(players, client_player, dataMutex);
    if (!playerAdded)
    {
        std::print(stderr, "Error: player attempted to be added to players while valid player was still there\n");
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        cleanupChildProcess(client_fd); // TODO: Make sure that client knows why they got booted
    }

    // TODO: Possibly make the shared resources obtained from the heap and use shared pointer or something
    // to pass it around so that it works properly with the different fork()s.

    // TODO: Add the new player to the list of clients atomically

    // TODO: If Hosting, create a 'lobby' in which they can play games.
    // Lobby can only have the two players, but do this so games can be repeated and
    // players can swap being the red and blue player

    // TODO: If joining, give a list of lobbies that need a second player.

    critical::addIDToQueue(freeIDs, client_id, dataMutex);
    cleanupChildProcess(client_fd);
}

int main()
{

    int serv_fd = networking::initServer();

    const std::size_t arraySize = static_cast<std::size_t>(UINT8_MAX) + 1;
    volatile std::array<Player, arraySize> players;
    volatile std::array<GameState, arraySize> gameStates;

    volatile std::queue<std::uint8_t> freeIDs = std::queue<std::uint8_t>();
    initializeFreeIDs(freeIDs, arraySize);

    std::mutex dataMutex;

    while (true)
    {
        int client_fd = networking::acceptConnection(serv_fd);
        if (client_fd == -1)
        {
            continue;
        }

        // TODO: change this to use threads instead of processes
        if (!fork()) // This is the child process associated with this player
        {
            bool client_disconnected = false;

            auto [client_id, noneAvailable] = critical::getAvailableID(freeIDs, IDsLockPtr);
            if (noneAvailable)
            {
                critical::addIDToQueue(freeIDs, client_id, IDsLockPtr);
                cleanupChildProcess(client_fd); // TODO: Make sure that client knows why they got booted
            }
            auto [client_player, disconnectedTmp0, isHosting] = matchmaking::getClientInfo(client_fd, client_id);
            client_disconnected = disconnectedTmp0;

            if (client_disconnected)
            {
                critical::addIDToQueue(freeIDs, client_id, IDsLockPtr);
                cleanupChildProcess(client_fd);
            }
            bool playerAdded = critical::addPlayerToPlayers(players, client_player, playersLockPtr);
            if (!playerAdded)
            {
                std::print(stderr, "Error: player attempted to be added to players while valid player was still there\n");
                critical::addIDToQueue(freeIDs, client_id, IDsLockPtr);
                cleanupChildProcess(client_fd); // TODO: Make sure that client knows why they got booted
            }

            // TODO: Possibly make the shared resources obtained from the heap and use shared pointer or something
            // to pass it around so that it works properly with the different fork()s.

            // TODO: Add the new player to the list of clients atomically

            // TODO: If Hosting, create a 'lobby' in which they can play games.
            // Lobby can only have the two players, but do this so games can be repeated and
            // players can swap being the red and blue player

            // TODO: If joining, give a list of lobbies that need a second player.

            critical::addIDToQueue(freeIDs, client_id, IDsLockPtr);
            cleanupChildProcess(client_fd);
        }
    }

    networking::closeFd(serv_fd);

    return 0;
}
