#include "GameState.hpp"
#include "Player.hpp"

// Networking Libraries
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

void reportErrno()
{
    std::print(stderr, "Error: {}\n", strerror(errno));
}

void setupDatabase() // NOTE: change return type to tuple or something once I figure out what
{
    // TODO: Make object that contains all the information about a player
    // Like name, ID, ip, port, red/blue
    const std::size_t arraySize = static_cast<std::size_t>(UINT8_MAX) + 1;
    std::array<Player, arraySize> players;
    std::array<GameState, arraySize> gameStates;
    // TODO: Make another object that contains game state, including who's turn it is, and the player IDs
}

int getLocalAddrInfo(const char*& port, addrinfo*& servinfo)
{
    addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    return getaddrinfo(nullptr, port, &hints, &servinfo);
}

void sigchld_handler(int s)
{
    (void)s; // this quiets unused parameter warning. Will remove later if still unused.

    int saved_errno = errno;
    while (waitpid(-1, nullptr, WNOHANG) > 0)
        ;
    errno = saved_errno;
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

        if (!fork()) // This is the child process
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
                std::print("Server: The client disconnected correctly.\n");
                break;
            }
        }
        close(client_fd); // Parent doesn't use this anymore, will stay open for child.

        // TODO: add code to handle a connection and handle the gameplay
    }
}

int main()
{
    const char* MYPORT = "3490";
    const int BACKLOG = 10;

    addrinfo* servinfo;
    int errCode;

    if ((errCode = getLocalAddrInfo(MYPORT, servinfo)) != 0)
    {
        std::print(stderr, "getaddrinfo: {}\n", gai_strerror(errCode));
        reportErrno();
    }

    int sockfd;
    int yes = 1;
    addrinfo* p;
    for (p = servinfo; p != nullptr; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
        {
            perror("server: socket");
            continue;
        }

        errCode = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (errCode == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        errCode = bind(sockfd, p->ai_addr, p->ai_addrlen);
        if (errCode == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == nullptr)
    {
        std::print("server: failed to bind\n");
        exit(1);
    }

    errCode = listen(sockfd, BACKLOG);
    if (errCode == -1)
    {
        perror("listen");
        exit(1);
    }

    // NOTE: This code works with sigaction() and reaps zombie processes.
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    errCode = sigaction(SIGCHLD, &sa, nullptr);
    if (errCode == -1)
    {
        perror("sigaction");
        exit(1);
    }

    std::print("server: waiting for connections...\n");
    // TODO: add code to set up the server and add to the main arguments
    mainAcceptLoop(sockfd);

    return 0;
}
