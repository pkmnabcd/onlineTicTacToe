#include <cstring>
#include <netdb.h>
#include <print>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void reportErrno()
{
    std::print(stderr, "Error: {}\n", strerror(errno));
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

    return 0;
}
