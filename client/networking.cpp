#include "networking.hpp"

#include "config.hpp"
#include "ConfigData.hpp"

#include <cstring>
#include <string>
#include <print>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#define MSG_NOSIGNAL 0

#else

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#endif


void* get_in_addr(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void networking::closeFd(SocketType fd)
{
    #ifdef _WIN32
    closesocket(fd);
    #else
    close(fd);
    #endif
}

// NOTE: this is only slightly modified from the Beej networking guide.
SocketType networking::initClient()
{
    ConfigData config = readConfig();
    // TODO: make sure these have \0 at the end
    const char* SERVER_PORT = config.m_serv_port.data();
    const char* SERVER_ADDRESS = config.m_serv_addr.data();

    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        //std::print(stderr, "WSAStartup failed.\n");
        return INVALID_SOCK_VAL;
    }
    #endif

    SocketType sockfd = INVALID_SOCK_VAL;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(SERVER_ADDRESS, SERVER_PORT, &hints, &servinfo)) != 0) {
        //std::print(stderr, "getaddrinfo: {}\n", gai_strerror(rv));
        return INVALID_SOCK_VAL;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            //perror("client: socket");
            continue;
        }

        inet_ntop(p->ai_family,
            get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
        std::print("Attempting connection to the server: {}\n", s);

        if (connect(sockfd, p->ai_addr, static_cast<int>(p->ai_addrlen)) == -1) {
            //perror("client: connect");
            networking::closeFd(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        return INVALID_SOCK_VAL;
    }

    inet_ntop(p->ai_family,
            get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);

    freeaddrinfo(servinfo); // all done with this structure
    return sockfd;
}


int networking::receiveAll(SocketType fd, char buffer[], int len)
{
    int bytesReceived = 0;
    while (bytesReceived != len)
    {
        // NOTE: char is 1B so 'bytesReceived' is # chars received
        const int newBytes = recv(fd, buffer+bytesReceived, len-bytesReceived, 0);
        if (newBytes == -1)
        {
            //perror("recv");
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

int networking::sendAll(SocketType fd, const char buffer[], int len)
{
    int bytesSent = 0;
    while (bytesSent != len)
    {
        // NOTE: char is 1B so 'bytesSent' is # chars sent
        // TODO: instead of the MSG_NOSIGNAL flag, use 0 when compiling for windows.
        const int newBytes = send(fd, buffer+bytesSent, len-bytesSent, MSG_NOSIGNAL);
        if (newBytes == -1)
        {
            //perror("send");
            bytesSent = newBytes;
            break;
        }
        if (newBytes == 0)
        {
            bytesSent = newBytes;
            break;
        }
        bytesSent += newBytes;
    }
    return bytesSent;
}

void networking::cleanup()
{
    #ifdef _WIN32
    WSACleanup();
    #endif
}
