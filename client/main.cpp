#include "../game/interface.hpp"
#include "../game/ttt.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"

#include <print>
#include <string>
// TODO: move the game folder to just the client folder since I didn't end up using game/ at all in the server.

void doMultiplayer()
{
    // TODO: change the game display/interface code to clear the screen
    // TODO: get code to get username, using character limit
    // and whether they want to host or join a lobby
    std::string username = interface::getUsername();
    bool hostGame = interface::selectHostLobby();
    int serv_fd = networking::initClient();

    bool message_sent_success;
    message_sent_success = matchmaking::sendPlayerInfo(serv_fd, hostGame, username);
    if (!message_sent_success)
    {
        std::print("Disconnected from server.\n");
        return;
    }
    std::print("Sent player info\n");

    bool disconnected;
    if (hostGame)
    {
        std::print("Trying to get confirmation\n");
        disconnected = matchmaking::getConfirmationMsg(serv_fd);
        if (disconnected)
        {
            std::print("Disconnected from server. Didn't receive lobby confirmation msg.\n");
            return;
        }
        std::print("Received confirmation msg\n");

        // Wait for a guest to join lobby and hear from server
        while (1)
        {
            message_sent_success = matchmaking::sendPing(serv_fd);
            if (!message_sent_success)
            {
                std::print("Disconnected from server while waiting for guest.\n");
                return;
            }

            auto [stillWaiting, disconnectedTmp0] = matchmaking::getWaitStatus(serv_fd);
            disconnected = disconnectedTmp0;
            if (disconnected)
            {
                std::print("Disconnected from server while waiting for guest.\n");
                return;
            }
            if (!stillWaiting)
            {
                break;
            }
        }
        std::print("No longer waiting for guest!");
    }

    networking::closeFd(serv_fd);
}

int main()
{
    bool keepPlaying = true;
    while (keepPlaying)
    {
        interface::logo();
        std::uint8_t mode = interface::selectGameMode();
        switch (mode)
        {
            case 0:
                runGame();
                break;
            case 1:
                doMultiplayer();
                break;
            case 'q':
                keepPlaying = false;
                break;
        }
    }

    return 0;
}
