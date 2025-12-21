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

void* get_in_addr(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void mainAcceptLoop(int self_fd)
{
    while (true)
    {
        socklen_t sin_size;
        sockaddr_storage client_addr;
        int client_fd;

        sin_size = sizeof client_addr;
        client_fd = accept(self_fd, (sockaddr*)&client_addr, &sin_size);
        if (client_fd == -1)
        {
            perror("accept");
            continue;
        }

        char s[INET6_ADDRSTRLEN];
        inet_ntop(client_addr.ss_family, get_in_addr((sockaddr*)&client_addr), s, sizeof s);
        std::print("server: got connection from {}\n", s);
        unsigned short int clientPort;
        if (client_addr.ss_family == AF_INET)
        {
            clientPort = ((sockaddr_in*)&client_addr)->sin_port;
        }
        else // is IPv6
        {
            clientPort = ((sockaddr_in6*)&client_addr)->sin6_port;
        }
        std::print("server: client port number: {}\n", clientPort);

        if (!fork()) // This is the child process associated with this player
        {
            close(self_fd); // Child does not need this, will stay open in main process.

            // Get initial info from client
            const int clientInfoBufferLen = 20;
            char clientInfoBuffer[clientInfoBufferLen];
            int numbytes = recv(client_fd, clientInfoBuffer, clientInfoBufferLen, 0);
            if (numbytes == -1)
            {
                perror("recv");
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

            close(client_fd);
            exit(0); // Kill the child process
        }
        close(client_fd); // Parent doesn't use this anymore, will stay open for child.

        // TODO: add code to handle a connection and handle the gameplay
    }
}

int main()
{

    int serv_fd = networking::initServer();

    // TODO: add code to set up the server and add to the main arguments
    mainAcceptLoop(serv_fd);

    networking::cleanup(serv_fd);

    return 0;
}
