#pragma once

namespace networking
{
    void closeFd(int fd);
    int initServer();
    int acceptConnection(int serv_fd);
} // namespace networking
