#include <cstring>
#include <netdb.h>
#include <print>
#include <sys/socket.h>
#include <sys/types.h>

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

    freeaddrinfo(servinfo);

    return 0;
}
