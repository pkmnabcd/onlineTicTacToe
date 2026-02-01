#include "matchmaking.hpp"
#include "networking.hpp"
#include "Player.hpp"

#include <cassert>
#include <cstdint>
#include <tuple>

const std::uint8_t NAME_LEN = 15; // NOTE: this includes terminating byte /0.


// TODO: see if I need to add 1 to each buffer len for \0

std::tuple<Player, bool, bool> matchmaking::getClientInfo(int client_fd, std::uint8_t client_id)
{
    bool disconnected = false;
    const int clientInfoBufferLen = NAME_LEN + 1;
    char clientInfoBuffer[clientInfoBufferLen];
    int numbytes = networking::receiveAll(client_fd, clientInfoBuffer, clientInfoBufferLen);

    // First char signifies whether client wants to host a game.
    // The other chars are the name the client picks
    // TODO: someday add limits to the names and check the current names to make sure it's unique
    std::string clientInfo(clientInfoBuffer);
    bool isHosting = clientInfo.at(0) == 'H';
    std::string client_name = clientInfo.substr(1);
    if (numbytes == -1 || numbytes == 0)
    {
        disconnected = true;
    }
    Player player = Player(client_name, client_id, client_fd);
    return std::make_tuple(player, disconnected, isHosting);
}

bool matchmaking::reportSuccessfulLobbyCreation(int client_fd)
{
    const int bufferLen = 8;
    char buffer[bufferLen] = "Success";
    int bytesSent = networking::sendAll(client_fd, buffer, bufferLen);

    return bytesSent == bufferLen;
}

bool matchmaking::sendHostTheGuestName(int client_fd, std::string guestName)
{
    assert(guestName.length() <= (NAME_LEN-1) && "Guest name not bigger than the maximum allowed name length");

    const int bufferLen = NAME_LEN;
    int bytesSent = networking::sendAll(client_fd, guestName.c_str(), bufferLen);
    return bytesSent == bufferLen;
}

std::tuple<bool, bool> matchmaking::hostChoosesRed(int client_fd)
{
    bool disconnected = false;
    const int chooseColorBufferLen = 1;
    char chooseColorBuffer[chooseColorBufferLen];
    int numbytes = networking::receiveAll(client_fd, chooseColorBuffer, chooseColorBufferLen);

    // First char signifies whether client wants to host a game.
    // The other chars are the name the client picks
    // TODO: someday add limits to the names and check the current names to make sure it's unique
    std::string colorChoice(chooseColorBuffer);
    bool choosesRed = colorChoice.at(0) == 'R';
    if (numbytes == -1 || numbytes == 0)
    {
        disconnected = true;
    }
    return std::make_tuple(choosesRed, disconnected);
}

bool matchmaking::sendGuestTheHostColor(int client_fd, bool hostChoseRed)
{
    const int bufferLen = 1;
    int bytesSent;
    if (hostChoseRed)
    {
        bytesSent = networking::sendAll(client_fd, "R", bufferLen);
    }
    else
    {
        bytesSent = networking::sendAll(client_fd, "B", bufferLen);
    }
    return bytesSent == bufferLen;
}
