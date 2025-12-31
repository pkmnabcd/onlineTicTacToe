#include "matchmaking.hpp"
#include "networking.hpp"
#include "Player.hpp"

#include <cstdint>
#include <tuple>

std::tuple<Player, bool, bool> matchmaking::getClientInfo(int client_fd, std::uint8_t client_id)
{
    bool disconnected = false;
    const int clientInfoBufferLen = 20;
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
