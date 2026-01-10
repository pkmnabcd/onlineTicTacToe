#include "matchmaking.hpp"
#include "networking.hpp"
#include "Player.hpp"

#include <cassert>
#include <cstdint>
#include <tuple>

const std::uint8_t nameLen = 15; // NOTE: this includes terminating byte /0.

std::tuple<Player, bool, bool> matchmaking::getClientInfo(int client_fd, std::uint8_t client_id)
{
    bool disconnected = false;
    const int clientInfoBufferLen = nameLen + 1;
    char clientInfoBuffer[clientInfoBufferLen];
    int numbytes = networking::receiveAll(client_fd, clientInfoBuffer, clientInfoBufferLen);

    // First char signifies whether client wants to host a game.
    // The other chars are the name the client picks
    // TODO: someday add limits to the names and check the current names to make sure it's unique
    std::string clientInfo(clientInfo);
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
    assert(guestName.length() <= (nameLen-1) && "Guest name not bigger than the maximum allowed name length");

    const int bufferLen = nameLen;
    int bytesSent = networking::sendAll(client_fd, guestName.c_str(), bufferLen);
    return bytesSent == bufferLen;
}
