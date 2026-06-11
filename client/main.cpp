#include "../game/interface.hpp"
#include "../game/ttt.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"

#include <print>
#include <string>

void doMultiplayer()
{
    // TODO: change the game display/interface code to clear the screen
    // TODO: get code to get username, using character limit
    // and whether they want to host or join a lobby
    std::string username = "tmpuser1234567"; // should be max char limit
    bool hostGame = true;
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
