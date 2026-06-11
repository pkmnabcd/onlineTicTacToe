#pragma once

#include "Lobby.hpp"
#include "Player.hpp"

#include <array>
#include <cstdint>
#include <tuple>
#include <vector>

using StraightBoard = std::array<std::string, 9>;

namespace matchmaking
{
    std::tuple<Player, bool, bool> getClientInfo(int client_fd, std::uint8_t client_id);
    bool reportSuccessfulLobbyCreation(int client_fd);
    bool sendHostTheGuestName(int client_fd, std::string guestName);
    std::tuple<bool, bool> hostChoosesRed(int client_fd);
    bool sendGuestTheHostColor(int client_fd, char hostColor);
    bool sendBoardState(int client_fd, StraightBoard board);
    std::tuple<std::uint8_t, bool> getClientMove(int client_fd);
    bool sendClientGameStatus(int client_fd, char winnerOrContOrOppDiscon);
    std::tuple<bool, bool> getClientPlayAgain(int client_fd);
    bool sendClientOppPlayAgain(int client_fd, bool oppPlayAgain);
    bool sendClientOpenLobbies(int client_fd, std::vector<Lobby> openLobbies);
    std::tuple<std::uint8_t, bool> getClientLobbyChoice(int client_fd);
    bool sendClientSuccessfulConnectionToLobby(int client_fd, bool successfulConnection);
    bool getClientCheckIn(int client_fd);
    bool sendCheckIn(int client_fd, bool stillWaiting);
} // namespace matchmaking
