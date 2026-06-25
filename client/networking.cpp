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
const char* SERVER_ADDRESS = "127.0.0.1";


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

void networking::closeFd(int fd)
{
    close(fd);
}

// NOTE: this is only slightly modified from the Beej networking guide.
int networking::initClient()
{

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(SERVER_ADDRESS, SERVER_PORT, &hints, &servinfo)) != 0) {
        std::print(stderr, "getaddrinfo: {}\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        inet_ntop(p->ai_family,
            get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
        std::print("client: attempting connection to {}\n", s);

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        std::print(stderr, "client: failed to connect\n");
        exit(1);
    }

    inet_ntop(p->ai_family,
            get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    std::print("client: connected to {}\n", s);

    freeaddrinfo(servinfo); // all done with this structure
    return sockfd;
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
            std::print("Server: The client disconnected.\n");
            bytesReceived = newBytes;
            break;
        }
        bytesReceived += newBytes;
    }
    return bytesReceived;
}

int networking::sendAll(int fd, const char buffer[], int len)
{
    int bytesSent = 0;
    while (bytesSent != len)
    {
        // NOTE: char is 1B so 'bytesSent' is # chars sent
        // TODO: instead of the MSG_NOSIGNAL flag, use 0 when compiling for windows.
        const int newBytes = send(fd, buffer+bytesSent, len-bytesSent, MSG_NOSIGNAL);
        if (newBytes == -1)
        {
            perror("send");
            bytesSent = newBytes;
            break;
        }
        if (newBytes == 0)
        {
            std::print("Server: Connection closed or unable to send.\n");
            bytesSent = newBytes;
            break;
        }
        bytesSent += newBytes;
    }
    return bytesSent;
}

