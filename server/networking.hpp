#pragma once

#ifdef _WIN32

    #include <winsock2.h>
    #include <ws2tcpip.h>

    #pragma comment(lib, "Ws2_32.lib")
    #define INVALID_SOCK_VAL INVALID_SOCKET

using SocketType = SOCKET;

#else

    #define INVALID_SOCK_VAL (-1)
using SocketType = int;

#endif

namespace networking
{
    void closeFd(SocketType fd);
    SocketType initServer();
    SocketType acceptConnection(SocketType serv_fd);
    int receiveAll(SocketType fd, char buffer[], int len);
    int sendAll(SocketType fd, const char buffer[], int len);
    void cleanup();
} // namespace networking
