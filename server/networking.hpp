#pragma once

namespace networking
{
    void closeFd(int fd);
    int initServer();
    int acceptConnection(int serv_fd);
    int receiveAll(int fd, char buf[], int len);
} // namespace networking
