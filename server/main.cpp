#include "GameState.hpp"
#include "Player.hpp"
#include "networking.hpp"

#include <arpa/inet.h>
#include <array>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <print>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void setupDatabase() // NOTE: change return type to tuple or something once I figure out what
{
    // TODO: Make object that contains all the information about a player
    // Like name, ID, ip, port, red/blue
    const std::size_t arraySize = static_cast<std::size_t>(UINT8_MAX) + 1;
    std::array<Player, arraySize> players;
    std::array<GameState, arraySize> gameStates;
    // TODO: Make another object that contains game state, including who's turn it is, and the player IDs
}

int main()
{

    int serv_fd = networking::initServer();

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

            // Get initial info from client
            const int clientInfoBufferLen = 20;
            char clientInfoBuffer[clientInfoBufferLen];
            int numbytes = networking::receiveAll(client_fd, clientInfoBuffer, clientInfoBufferLen);
            if (numbytes == -1)
            {
                break;
            }
            if (numbytes == 0)
            {
                std::print("Server: The client disconnected.\n");
                break;
            }

            // NOTE: At some point I need to move most code in this file to a networking-specific
            // .cpp file so that I don't have to worry about all the networking stuff all the time

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
