#pragma once

#include "Player.hpp"

#include <cstdint>
#include <tuple>

namespace matchmaking
{
    std::tuple<Player, bool, bool> getClientInfo(int client_fd, std::uint8_t client_id);
    bool reportSuccessfulLobbyCreation(int client_fd);
    bool sendHostTheGuestName(int client_fd, std::string guestName);
    std::tuple<bool, bool> hostChoosesRed(int client_fd);
    bool sendGuestTheHostColor(int client_fd, bool hostChoseRed);
} // namespace matchmaking
