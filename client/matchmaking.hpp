#pragma once

#include "networking.hpp"
#include "util.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

using StraightBoard = std::array<char, 9>;
using Board = std::array<std::array<char, 3>, 3>;

namespace matchmaking
{
    bool sendPlayerInfo(SocketType serv_fd, bool hostGame, std::string username);
    std::tuple<std::uint8_t, bool> getYourID(SocketType serv_fd);
    std::tuple<bool, bool> getWaitStatus(SocketType serv_fd);
    bool sendPing(SocketType serv_fd);
    std::tuple<std::string, bool> getGuestName(SocketType serv_fd);
    std::tuple<std::vector<std::tuple<std::string, std::uint8_t>>, bool> getOpenLobbies(SocketType serv_fd);
    bool sendLobbyChoice(SocketType serv_fd, std::uint8_t hostID);
    std::tuple<bool, bool> getLobbyConnectionSuccessConfirmation(SocketType serv_fd);
    bool sendColorChoice(SocketType serv_fd, bool choseRed);
    bool blockAndPing(SocketType serv_fd);
    std::tuple<bool, bool, bool> getHostColor(SocketType serv_fd);
    std::tuple<bool, util::Winner, bool, bool> getGameStatus(SocketType serv_fd);
    Board getBoardFromStraightBoard(StraightBoard straightBoard);
    std::tuple<Board, bool> getBoardState(SocketType serv_fd);
    bool sendMove(SocketType serv_fd, std::uint8_t move);
    bool sendPlayAgain(SocketType serv_fd, bool playAgain);
    std::tuple<bool, bool> getOppPlayAgain(SocketType serv_fd);
} // namespace matchmaking
