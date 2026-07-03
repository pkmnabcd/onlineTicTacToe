#pragma once

#include "Lobby.hpp"
#include "Player.hpp"
#include "networking.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <tuple>
#include <vector>

using StraightBoard = std::array<std::string, 9>;

namespace matchmaking
{
    std::tuple<Player, bool, bool> getClientInfo(SocketType client_fd, std::uint8_t client_id);
    bool sendClientID(SocketType client_fd, std::uint8_t client_id);
    bool sendHostTheGuestName(SocketType client_fd, std::string guestName);
    std::tuple<bool, bool> hostChoosesRed(SocketType client_fd);
    bool sendGuestTheHostColor(SocketType client_fd, char hostColor);
    bool sendBoardState(SocketType client_fd, StraightBoard board);
    std::tuple<std::uint8_t, bool, bool> getClientMove(SocketType client_fd);
    bool sendClientGameStatus(SocketType client_fd, char winnerOrContOrOppDiscon);
    std::tuple<bool, bool> getClientPlayAgain(SocketType client_fd);
    bool sendClientOppPlayAgain(SocketType client_fd, bool oppPlayAgain);
    bool sendClientOpenLobbies(SocketType client_fd, std::vector<Lobby> openLobbies);
    std::tuple<std::uint8_t, bool> getClientLobbyChoice(SocketType client_fd);
    bool sendClientSuccessfulConnectionToLobby(SocketType client_fd, bool successfulConnection);
    bool getClientCheckIn(SocketType client_fd);
    bool sendCheckIn(SocketType client_fd, bool stillWaiting);
    bool blockUntilCondition(SocketType client_fd, std::function<bool()> condition);
} // namespace matchmaking
