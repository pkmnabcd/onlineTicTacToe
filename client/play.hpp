#pragma once

#include <string>
#include <tuple>

namespace play
{
    std::tuple<bool, bool, bool> playGame(int serv_fd, bool isRed, std::string oppName);
} // namespace play
