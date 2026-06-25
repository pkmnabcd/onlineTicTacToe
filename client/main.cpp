#include "interface.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"
#include "ttt.hpp"

#include <print>
#include <string>

void doMultiplayer()
{
    // TODO: Possibly change the game display/interface code to clear the screen.
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
        auto [yourID, disconnectedTmp0] = matchmaking::getYourID(serv_fd);
        disconnected = disconnectedTmp0;
        if (disconnected)
        {
            std::print("Disconnected from server. Didn't receive your ID.\n");
            return;
        }
        std::print("Your Online ID for this session is: {}\n", yourID);

        // Wait for a guest to join lobby and hear from server
        while (1)
        {
            message_sent_success = matchmaking::sendPing(serv_fd);
            if (!message_sent_success)
            {
                std::print("Disconnected from server while waiting for guest.\n");
                return;
            }

            auto [stillWaiting, disconnectedTmp1] = matchmaking::getWaitStatus(serv_fd);
            disconnected = disconnectedTmp1;
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
        std::print("No longer waiting for guest!\n");

        auto [guestName, disconnectedTmp2] = matchmaking::getGuestName(serv_fd);
        disconnected = disconnectedTmp2;
        if (disconnected)
        {
            std::print("Disconnected from server while getting guest name.\n");
            return;
        }
        std::print("You have connected with guest: {}\n", guestName);

        // TODO: Stuff to do:
        // 1. Choose between being red or blue
        // 2. Game logic
        // 3. ...
        bool choseRed = interface::chooseRedOrBlue();
        message_sent_success = matchmaking::sendLobbyChoice(serv_fd, choseRed);
        if (!message_sent_success)
        {
            std::print("Disconnected from server while sending color choice.\n");
            return;
        }
    }
    else // Player wants to join an existing game
    {
        auto [lobbies, disconnectedTmp0] = matchmaking::getOpenLobbies(serv_fd);
        disconnected = disconnectedTmp0;
        if (disconnected)
        {
            std::print("Disconnected from server while trying to get the open lobbies.\n");
            return;
        }

        std::uint8_t lobbyHostID = interface::chooseLobby(lobbies);
        message_sent_success = matchmaking::sendLobbyChoice(serv_fd, lobbyHostID);
        if (!message_sent_success)
        {
            std::print("Disconnected from server while sending lobby choice.\n");
            return;
        }
        // TODO: Stuff to do:
        // 1. Recieve the color of your opponent
        // 3. Game logic
        // 2. ...

        auto [connectionSuccess, disconnectedTmp1] = matchmaking::getLobbyConnectionSuccessConfirmation(serv_fd);
        disconnected = disconnectedTmp1;
        if (disconnected)
        {
            std::print("Disconnected from server while confirming lobby selection.\n");
            return;
        }
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
