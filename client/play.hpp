#pragma once

#include "networking.hpp"

#include <string>
#include <tuple>

namespace play
{
    std::tuple<bool, bool, bool> playGame(SocketType serv_fd, bool isRed, std::string oppName);
} // namespace play
