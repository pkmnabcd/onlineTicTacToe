#include <arpa/inet.h>
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

void mainAcceptLoop(int sockfd)
{
    while (true)
    {
        socklen_t sin_size;
        struct sockaddr_storage client_addr;
        int new_fd;

        sin_size = sizeof client_addr;
        new_fd = accept(sockfd, (struct sockaddr*)&client_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        char s[INET6_ADDRSTRLEN];
        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr*)&client_addr), s, sizeof s);
        std::print("server: got connection from {}\n", s);

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
