#include "GameState.hpp"
#include "Player.hpp"
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

int main()
{

    int serv_fd = networking::initServer();

    const std::size_t arraySize = static_cast<std::size_t>(UINT8_MAX) + 1;
    std::array<Player, arraySize> players;
    std::array<GameState, arraySize> gameStates;

    std::queue<std::uint8_t> freeIDs = std::queue<std::uint8_t>();
    initializeFreeIDs(freeIDs, arraySize);

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

            // TODO: Check whether freeIDs is empty and close up if it is
            std::uint8_t client_id = freeIDs.front();
            freeIDs.pop();
            // TODO: change this to structured binding syntax and add third item that checks if hosting
            std::tuple<Player, bool> infoResult = matchmaking::getClientInfo(client_fd, client_id);

            Player client_player = std::get<0>(infoResult);
            client_disconnected = std::get<1>(infoResult);
            if (client_disconnected)
            {
                networking::closeFd(client_fd);
                exit(0); // Kill child process
            }

            // TODO: Get from client info (first char?) whether the client is hosting a match or
            // if they're joinging someone else. Also assign ID and add to player list

            // TODO: If Hosting, create a 'lobby' in which they can play games.
            // Lobby can only have the two players, but do this so games can be repeated and
            // players can swap being the red and blue player

            // TODO: If joining, give a list of lobbies that need a second player.

            networking::closeFd(client_fd);
            exit(0); // Kill the child process
        }
        networking::closeFd(client_fd); // Parent doesn't use this anymore, will stay open for child.
    }

    networking::closeFd(serv_fd);

    return 0;
}
