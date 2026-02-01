#pragma once

#include "Player.hpp"

#include <array>
#include <cstdint>
#include <tuple>

using StraightBoard = std::array<std::string, 9>;

namespace matchmaking
{
    std::tuple<Player, bool, bool> getClientInfo(int client_fd, std::uint8_t client_id);
    bool reportSuccessfulLobbyCreation(int client_fd);
    bool sendHostTheGuestName(int client_fd, std::string guestName);
    std::tuple<bool, bool> hostChoosesRed(int client_fd);
    bool sendGuestTheHostColor(int client_fd, bool hostChoseRed);
    bool sendBoardState(int client_fd, StraightBoard board);
} // namespace matchmaking
