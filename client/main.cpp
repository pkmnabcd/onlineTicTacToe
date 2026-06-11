#include "../game/interface.hpp"
#include "../game/ttt.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"

#include <print>
#include <string>

void doMultiplayer()
{
    // TODO: get code to get username, using character limit
    // and whether they want to host or join a lobby
    std::string username = "tmpuser1234567"; // should be max char limit
    bool hostGame = true;
    int serv_fd = networking::initClient();

    bool disconnected;
    disconnected = matchmaking::sendPlayerInfo(serv_fd, hostGame, username);
    std::print("Sent player info");

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
