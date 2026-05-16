#pragma once

namespace networking
{
    void closeFd(int fd);
    int initClient();
    int receiveAll(int fd, char buffer[], int len);
    int sendAll(int fd, const char buffer[], int len);
} // namespace networking
