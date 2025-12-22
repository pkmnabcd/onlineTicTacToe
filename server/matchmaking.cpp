#include "matchmaking.hpp"
#include "networking.hpp"
#include "Player.hpp"

#include <cstdint>
#include <tuple>

// TODO: Update to decide where/how ID is determined
std::tuple<Player, bool> matchmaking::getClientInfo(int client_fd, std::uint8_t client_id)
{
    bool disconnected = false;
    const int clientInfoBufferLen = 20;
    char clientInfoBuffer[clientInfoBufferLen];
    int numbytes = networking::receiveAll(client_fd, clientInfoBuffer, clientInfoBufferLen);
    if (numbytes == -1 || numbytes == 0)
    {
        disconnected = true;
    }
    // TODO: Update how to get player name
    Player player = Player("update me", client_id, client_fd);
    return std::make_tuple(player, disconnected);
}
