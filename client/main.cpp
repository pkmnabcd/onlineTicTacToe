#include "interface.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"
#include "play.hpp"
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

        bool wantsToPlay = true;
        while (wantsToPlay)
        {
            std::print("Your Online ID for this session is: {}\n", yourID);

            // Wait for a guest to join lobby and hear from server
            disconnected = matchmaking::blockAndPing(serv_fd);
            if (disconnected)
            {
                std::print("Disconnected from server while waiting for guest.\n");
                return;
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

            bool choseRed = interface::chooseRedOrBlue();
            message_sent_success = matchmaking::sendLobbyChoice(serv_fd, choseRed);
            if (!message_sent_success)
            {
                std::print("Disconnected from server while sending color choice.\n");
                return;
            }

            // TODO: Stuff to do for host:
            // 1. Game logic
            // 2. ...
            auto [playAgainTmp, disconnectedTmp3, oppDisconnected] = play::playGame(serv_fd, choseRed);
            wantsToPlay = playAgainTmp;
            disconnected = disconnectedTmp3;
            return; // TODO: remove this. It's just here to not mess with the server logic while it's incomplete.
        }
    }
    else // Player wants to join an existing game
    {
        bool wantsToPlay = true;
        while (wantsToPlay)
        {
            auto [lobbies, disconnectedTmp0] = matchmaking::getOpenLobbies(serv_fd);
            disconnected = disconnectedTmp0;
            if (disconnected)
            {
                std::print("Disconnected from server while trying to get the open lobbies.\n");
                return;
            }
            if (lobbies.size() == 0)
            {
                std::print("No open lobbies. Host a lobby or try again later.\n");
                return;
            }

            std::uint8_t lobbyHostID = interface::chooseLobby(lobbies);
            message_sent_success = matchmaking::sendLobbyChoice(serv_fd, lobbyHostID);
            if (!message_sent_success)
            {
                std::print("Disconnected from server while sending lobby choice.\n");
                return;
            }

            auto [connectionSuccess, disconnectedTmp1] = matchmaking::getLobbyConnectionSuccessConfirmation(serv_fd);
            disconnected = disconnectedTmp1;
            if (disconnected)
            {
                std::print("Disconnected from server while confirming lobby selection.\n");
                return;
            }

            disconnected = matchmaking::blockAndPing(serv_fd);
            if (disconnected)
            {
                std::print("Disconnected from server while waiting for the host to pick color.\n");
                return;
            }
            auto [hostChoseRed, hostDisconnected, disconnectedTmp2] = matchmaking::getHostColor(serv_fd);
            disconnected = disconnectedTmp2;
            if (disconnected)
            {
                std::print("Disconnected from server while getting the host's color choice.\n");
                return;
            }
            if (hostDisconnected)
            {
                std::print("The lobby host disconnected.\n");
                continue; // Go back to choosing a lobby
            }
            std::print("Host chose {}.\n", (hostChoseRed) ? "red" : "blue");

            // TODO: Stuff to do for guest:
            // 1. Game logic
            // 2. ...
            auto [playAgainTmp, disconnectedTmp3, oppDisconnected] = play::playGame(serv_fd, !hostChoseRed);
            wantsToPlay = playAgainTmp;
            disconnected = disconnectedTmp3;
            return; // TODO: remove this. It's just here to not mess with the server logic while it's incomplete.
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
