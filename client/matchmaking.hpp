#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

using StraightBoard = std::array<std::string, 9>;

namespace matchmaking
{
    bool sendPlayerInfo(int serv_fd, bool hostGame, std::string username);
} // namespace matchmaking
