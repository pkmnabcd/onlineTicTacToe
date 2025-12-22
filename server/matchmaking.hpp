#pragma once

#include "Player.hpp"

#include <cstdint>
#include <tuple>

namespace matchmaking
{
    std::tuple<Player, bool> getClientInfo(int client_fd, std::uint8_t client_id);
}
