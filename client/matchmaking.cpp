#include "matchmaking.hpp"

#include "networking.hpp"

#include <cassert>
#include <cstdint>
#include <string>

const std::uint8_t NAME_LEN = 15; // NOTE: this includes terminating byte /0.

bool matchmaking::sendPlayerInfo(int serv_fd, bool hostGame, std::string username)
{
    const int playerInfoBufferLen = NAME_LEN + 1;
    char playerInfoBuffer[playerInfoBufferLen] = "";
    playerInfoBuffer[0] = (hostGame) ? 'H' : 'G';
    assert(username.length() < NAME_LEN && "Guest name should have length less than NAME_LEN");
    for (std::uint8_t i = 0; i < username.length(); i++)
    {
        playerInfoBuffer[i + 1] = username.at(i);
    }

    int bytesSent = networking::sendAll(serv_fd, playerInfoBuffer, playerInfoBufferLen);
    return bytesSent == playerInfoBufferLen;
}
