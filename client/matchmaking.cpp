#include "matchmaking.hpp"

#include "networking.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

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

bool matchmaking::getConfirmationMsg(int serv_fd)
{
    bool disconnected = false;

    const int bufferLen = 8;
    char buffer[bufferLen] = "";
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0)
    {
        disconnected = true;
    }
    else
    {
        char successStr[bufferLen] = "Success";
        disconnected = std::string_view(buffer) != std::string_view(successStr);
    }
    return disconnected;
}

std::tuple<bool, bool> matchmaking::getWaitStatus(int serv_fd)
{
    bool stillWaiting = true;
    bool disconnected = false;

    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0)
    {
        disconnected = true;
    }
    else
    {
        if (buffer[0] == 'W')
        {
            stillWaiting = true;
        }
        else if (buffer[0] == 'R')
        {
            stillWaiting = false;
        }
        else
        {
            disconnected = true;
        }
    }
    return std::make_tuple(stillWaiting, disconnected);
}

bool matchmaking::sendPing(int serv_fd)
{
    const int bufferLen = 2;
    char buffer[bufferLen] = "Y";
    int bytesSent = networking::sendAll(serv_fd, buffer, bufferLen);
    return bytesSent == bufferLen;
}

std::tuple<std::string, bool> matchmaking::getGuestName(int serv_fd)
{
    std::string guestName = "";
    bool disconnected = false;

    const int bufferLen = NAME_LEN;
    char buffer[bufferLen] = "";
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0)
    {
        disconnected = true;
    }
    else
    {
        if (buffer[NAME_LEN-1] != '\0')
        {
            disconnected = true;
        }
        else
        {
            guestName = std::string(buffer);
        }
    }
    return std::make_tuple(guestName, disconnected);
}

std::tuple<std::vector<std::tuple<std::string, std::uint8_t>>, bool> matchmaking::getOpenLobbies(int serv_fd)
{
    // TODO: Process for receiving the lobbies:
    // 1. Set up a buffers to recieve the lobby data into, but still read one char at a time.
    // 2. First look for the \x01 byte and then read the rest of the data. If it's \x02 or otherwise,
    // then report a failure or stop reading the lobbies.
    // 3. If \x01, read the first three chars and convert to int and then uint8_t, then read name (15 chars)
    // 4. Repeat until \x01 no longer found.
    std::vector<std::tuple<std::string, std::uint8_t>> openLobbies;
    bool disconnected = false;

    const int bufferLen = 1;
    char buffer[bufferLen] = "";
    while(1)
    {
        int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
        if (numbytes == -1 || numbytes == 0)
        {
            disconnected = true;
            break;
        }
    }
    return std::make_tuple(openLobbies, disconnected);
}
