#include "networking.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <print>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

const char* SERVER_PORT = "3490";
const int BACKLOG = 10;



int getLocalAddrInfo(const char*& port, addrinfo*& servinfo)
{
    addrinfo servInfoHints;
    memset(&servInfoHints, 0, sizeof servInfoHints);
    servInfoHints.ai_family = AF_UNSPEC;
    servInfoHints.ai_socktype = SOCK_STREAM;
    servInfoHints.ai_flags = AI_PASSIVE;

    return getaddrinfo(nullptr, port, &servInfoHints, &servinfo);
}

void sigchld_handler(int s)
{
    (void)s; // this quiets unused parameter warning.

    const int saved_errno = errno;
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

unsigned short int get_in_port(sockaddr_storage client_addr)
{
    if (client_addr.ss_family == AF_INET)
    {
        return ((sockaddr_in*)&client_addr)->sin_port;
    }
    else // is IPv6
    {
        return ((sockaddr_in6*)&client_addr)->sin6_port;
    }
}

void networking::closeFd(int fd)
{
    close(fd);
}

int networking::initServer()
{
    addrinfo* servinfo;
    int errCode;

    if ((errCode = getLocalAddrInfo(SERVER_PORT, servinfo)) != 0)
    {
        std::print(stderr, "getaddrinfo: {}\n", gai_strerror(errCode));
        perror("Server: getaddrinfo");
    }

    int serv_fd;
    int yes = 1;
    addrinfo* p;
    for (p = servinfo; p != nullptr; p = p->ai_next)
    {
        serv_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (serv_fd == -1)
        {
            perror("server: socket");
            continue;
        }

        errCode = setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (errCode == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        errCode = bind(serv_fd, p->ai_addr, p->ai_addrlen);
        if (errCode == -1)
        {
            close(serv_fd);
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

    errCode = listen(serv_fd, BACKLOG);
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
    return serv_fd;
}


int networking::acceptConnection(int serv_fd)
{
    socklen_t sin_size;
    sockaddr_storage client_addr;
    int client_fd;

    sin_size = sizeof client_addr;
    client_fd = accept(serv_fd, (sockaddr*)&client_addr, &sin_size);
    if (client_fd == -1)
    {
        perror("accept");
        return client_fd;
    }

    char s[INET6_ADDRSTRLEN];
    inet_ntop(client_addr.ss_family, get_in_addr((sockaddr*)&client_addr), s, sizeof s);
    std::print("server: got connection from {}\n", s);
    unsigned short int clientPort = get_in_port(client_addr);
    std::print("server: client port number: {}\n", clientPort);

    return client_fd;
}

int networking::receiveAll(int fd, char buffer[], int len)
{
    int bytesReceived = 0;
    while (bytesReceived != len)
    {
        // NOTE: char is 1B so 'bytesReceived' is # chars received
        const int newBytes = recv(fd, buffer+bytesReceived, len-bytesReceived, 0);
        if (newBytes == -1)
        {
            perror("recv");
            bytesReceived = newBytes;
            break;
        }
        if (newBytes == 0)
        {
            bytesReceived = newBytes;
            break;
        }
        bytesReceived += newBytes;
    }
    return bytesReceived;
}

