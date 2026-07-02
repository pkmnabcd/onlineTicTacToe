#include "interface.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"
#include "play.hpp"
#include "ttt.hpp"

#include <print>
#include <string>

std::string getCleanedName(std::string& name)
{
    /*
     * This function gets rid of the trailing spaces in the hostName string
     */
    std::string::size_type index;
    index = name.find_first_of(' ');
    if (index == std::string::npos)
    {
        return name;
    }
    return name.substr(0, index); // index should also be the number of chars before the spaces start
}

void doMultiplayer()
{
    // TODO: Possibly change the game display/interface code to clear the screen.
    std::string username = interface::getUsername();
    bool hostGame = interface::selectHostLobby();
    SocketType serv_fd = networking::initClient();
    if (serv_fd == INVALID_SOCK_VAL)
    {
        std::print("Failed to connect to server.\n");
        networking::cleanup();
        return;
    }
    std::print("Successful connection to server\n");

    bool message_sent_success;
    message_sent_success = matchmaking::sendPlayerInfo(serv_fd, hostGame, username);
    if (!message_sent_success)
    {
        std::print("Disconnected from server.\n");
        networking::closeFd(serv_fd);
        networking::cleanup();
        return;
    }

    bool disconnected;
    if (hostGame)
    {
        auto [yourID, disconnectedTmp0] = matchmaking::getYourID(serv_fd);
        disconnected = disconnectedTmp0;
        if (disconnected)
        {
            std::print("Disconnected from server. Didn't receive your ID.\n");
            networking::closeFd(serv_fd);
            networking::cleanup();
            return;
        }

        bool wantsToPlay = true;
        while (wantsToPlay)
        {
            std::print("Waiting for a guest. Your Online ID for this session is: {}\n", yourID);

            // Wait for a guest to join lobby and hear from server
            disconnected = matchmaking::blockAndPing(serv_fd);
            if (disconnected)
            {
                std::print("Disconnected from server while waiting for guest.\n");
                networking::closeFd(serv_fd);
                networking::cleanup();
                return;
            }

            auto [guestName, disconnectedTmp2] = matchmaking::getGuestName(serv_fd);
            disconnected = disconnectedTmp2;
            if (disconnected)
            {
                std::print("Disconnected from server while getting guest name.\n");
                networking::closeFd(serv_fd);
                networking::cleanup();
                return;
            }
            std::print("You have connected with guest: {}\n", guestName);

            bool oppWantsPlay = true;
            while (oppWantsPlay)
            {
                auto [choseRed, wantsQuit] = interface::chooseRedOrBlue();
                if (wantsQuit)
                {
                    networking::closeFd(serv_fd);
                    networking::cleanup();
                    return;
                }
                message_sent_success = matchmaking::sendColorChoice(serv_fd, choseRed);
                if (!message_sent_success)
                {
                    std::print("Disconnected from server while sending color choice.\n");
                    networking::closeFd(serv_fd);
                    networking::cleanup();
                    return;
                }

                auto [playAgainTmp, disconnectedTmp3, oppDisconnected] = play::playGame(serv_fd, choseRed, guestName);
                wantsToPlay = playAgainTmp;
                disconnected = disconnectedTmp3;
                if (disconnected || !wantsToPlay)
                {
                    networking::closeFd(serv_fd);
                    networking::cleanup();
                    return;
                }

                // Handle opponent response
                std::print("Waiting for the guest's decision.\n");
                auto [oppPlayAgainTmp0, disconnectedTmp4] = matchmaking::getOppPlayAgain(serv_fd);
                oppWantsPlay = oppPlayAgainTmp0;
                disconnected = disconnectedTmp4;
                if (disconnected)
                {
                    std::print("Disconnected while waiting for opponent to send whether they want to replay.\n");
                    networking::closeFd(serv_fd);
                    networking::cleanup();
                    return;
                }
                if (!oppWantsPlay)
                {
                    std::print("{} no longer wants to play.\n", guestName);
                }
            }
            if (wantsToPlay)
            {
                std::print("Waiting for a new player to connect.\n");
            }
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
                networking::closeFd(serv_fd);
                networking::cleanup();
                return;
            }
            if (lobbies.size() == 0)
            {
                std::print("No open lobbies. Host a lobby or try again later.\n");
                networking::closeFd(serv_fd);
                networking::cleanup();
                return;
            }

            auto [lobbyHostID, hostName] = interface::chooseLobby(lobbies);
            hostName = getCleanedName(hostName);
            message_sent_success = matchmaking::sendLobbyChoice(serv_fd, lobbyHostID);
            if (!message_sent_success)
            {
                std::print("Disconnected from server while sending lobby choice.\n");
                networking::closeFd(serv_fd);
                networking::cleanup();
                return;
            }

            auto [connectionSuccess, disconnectedTmp1] = matchmaking::getLobbyConnectionSuccessConfirmation(serv_fd);
            disconnected = disconnectedTmp1;
            if (disconnected)
            {
                std::print("Disconnected from server while confirming lobby selection or the lobby host disconnected.\n");
                networking::closeFd(serv_fd);
                networking::cleanup();
                return;
            }
            if (!connectionSuccess)
            {
                std::print("Lobby host disconnected. Searching again for open lobbies.\n");
                continue;
            }
            std::print("\nConnected to the lobby.\n");

            bool oppWantsPlay = true;
            while (oppWantsPlay)
            {
                std::print("Waiting for the host to choose red or blue.\n");
                disconnected = matchmaking::blockAndPing(serv_fd);
                if (disconnected)
                {
                    std::print("Disconnected from server while waiting for the host to pick color.\n");
                    networking::closeFd(serv_fd);
                    networking::cleanup();
                    return;
                }
                auto [hostChoseRed, hostDisconnected, disconnectedTmp2] = matchmaking::getHostColor(serv_fd);
                disconnected = disconnectedTmp2;
                if (disconnected)
                {
                    std::print("Disconnected from server while getting the host's color choice.\n");
                    networking::closeFd(serv_fd);
                    networking::cleanup();
                    return;
                }
                if (hostDisconnected)
                {
                    std::print("The lobby host disconnected.\n");
                    break; // Go back to choosing a lobby
                }
                std::print("Host chose {}.\n", (hostChoseRed) ? "red" : "blue");

                auto [playAgainTmp, disconnectedTmp3, oppDisconnected] = play::playGame(serv_fd, !hostChoseRed, hostName);
                wantsToPlay = playAgainTmp;
                disconnected = disconnectedTmp3;
                if (disconnected || !wantsToPlay)
                {
                    networking::closeFd(serv_fd);
                    networking::cleanup();
                    return;
                }

                // Handle opponent response
                std::print("Waiting for the host's decision.\n");
                auto [oppPlayAgainTmp0, disconnectedTmp4] = matchmaking::getOppPlayAgain(serv_fd);
                oppWantsPlay = oppPlayAgainTmp0;
                disconnected = disconnectedTmp4;
                if (disconnected)
                {
                    std::print("Disconnected while waiting for opponent to send whether they want to replay.\n");
                    networking::closeFd(serv_fd);
                    networking::cleanup();
                    return;
                }
                if (!oppWantsPlay)
                {
                    std::print("{} no longer wants to play.\n", hostName);
                }
            }
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
