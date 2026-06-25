#pragma once

#include <tuple>

namespace play
{
    std::tuple<bool, bool, bool> playGame(int serv_fd, bool isRed);
} // namespace play
