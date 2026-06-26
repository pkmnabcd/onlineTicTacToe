#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

using StraightBoard = std::array<std::string, 9>;
using Board = std::array<std::array<std::string, 3>, 3>;

namespace matchmaking
{
    enum Winner
    {
        Stalemate,
        Red,
        Blue,
        Undecided
    };

    bool sendPlayerInfo(int serv_fd, bool hostGame, std::string username);
    std::tuple<std::uint8_t, bool> getYourID(int serv_fd);
    std::tuple<bool, bool> getWaitStatus(int serv_fd);
    bool sendPing(int serv_fd);
    std::tuple<std::string, bool> getGuestName(int serv_fd);
    std::tuple<std::vector<std::tuple<std::string, std::uint8_t>>, bool> getOpenLobbies(int serv_fd);
    bool sendLobbyChoice(int serv_fd, std::uint8_t hostID);
    std::tuple<bool, bool> getLobbyConnectionSuccessConfirmation(int serv_fd);
    bool sendLobbyChoice(int serv_fd, bool choseRed);
    bool blockAndPing(int serv_fd);
    std::tuple<bool, bool, bool> getHostColor(int serv_fd);
    std::tuple<bool, matchmaking::Winner, bool, bool> getGameStatus(int serv_fd);
    std::tuple<Board, bool> getBoardState(int serv_fd);
    bool sendMove(int serv_fd, char move);
} // namespace matchmaking
