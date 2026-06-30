#include "GameState.hpp"
#include "Lobby.hpp"
#include "Player.hpp"
#include "critical.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"
#include "play.hpp"
#include "settings.hpp"

#include <array>
#include <chrono>
#include <functional>
#include <mutex>
#include <print>
#include <queue>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <vector>

void initializeFreeIDs(std::queue<std::uint8_t>& freeIDsQueue, std::size_t IDCount)
{
    for (std::size_t i = 0; i < IDCount; i++)
    {
        freeIDsQueue.push(i);
    }
}

void manageClient(int client_fd, std::array<Player, arraySize>& players, std::array<GameState, arraySize>& gamestates, std::array<Lobby, arraySize>& lobbies, std::queue<std::uint8_t>& freeIDs, std::mutex& dataMutex, std::mutex& disconnectMutex, std::array<std::mutex, arraySize>& gameMutexes)
{
    bool client_disconnected = false;
    bool message_sent_success;

    auto [client_id, noneAvailable] = critical::getAvailableID(freeIDs, dataMutex);
    if (noneAvailable)
    {
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        networking::closeFd(client_fd);
        return;
    }
    auto [client_player, disconnectedTmp0, isHosting] = matchmaking::getClientInfo(client_fd, client_id);
    client_disconnected = disconnectedTmp0;

    if (client_disconnected)
    {
        critical::invalidatePlayer(players, client_id, dataMutex);
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        networking::closeFd(client_fd);
        return;
    }
    // TODO: remove debugging statement
    std::print("Username {} wants to host {}\n", client_player.m_name, isHosting);
    const bool playerAdded = critical::addPlayerToPlayers(players, client_player, dataMutex);
    if (!playerAdded)
    {
        std::print(stderr, "Error: player attempted to be added to players while valid player was still there\n");
        critical::invalidatePlayer(players, client_id, dataMutex);
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        networking::closeFd(client_fd);
        return;
    }

    if (isHosting)
    {
        Lobby client_lobby = Lobby(client_player);
        message_sent_success = matchmaking::sendClientID(client_fd, client_id);
        if (!message_sent_success)
        {
            std::print(stderr, "Error: message send unsucessful\n");
            critical::invalidatePlayer(players, client_id, dataMutex);
            critical::addIDToQueue(freeIDs, client_id, dataMutex);
            networking::closeFd(client_fd);
            return;
        }

        const bool lobbyAdded = critical::addLobbyToLobbies(lobbies, client_lobby, dataMutex);
        if (!lobbyAdded)
        {
            std::print(stderr, "Error: lobby attempted to be added to lobbies while valid lobby was still there\n");
            critical::invalidatePlayer(players, client_id, dataMutex);
            critical::addIDToQueue(freeIDs, client_id, dataMutex);
            networking::closeFd(client_fd);
            return;
        }

        bool hostWantsToPlay = true;
        while (hostWantsToPlay)
        {
            // Wait until someone joins the lobby
            Player guest;
            std::function<bool()> guestJoinedCondition = [&]()
            {
                if (lobbies[client_id].m_guest.m_isValid)
                {
                    // NOTE: guest is modified inside the blockUntilCondition function.
                    guest = critical::getGuestFromClientLobby(lobbies, client_id, dataMutex);
                    return guest.m_isValid;
                }
                return false;
            };
            client_disconnected = matchmaking::blockUntilCondition(client_fd, guestJoinedCondition);
            if (client_disconnected)
            {
                std::print(stderr, "Error: guest disconnected while waiting for a guest.\n");
                critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, client_id, dataMutex, disconnectMutex);
                critical::invalidatePlayer(players, client_id, dataMutex);
                critical::addIDToQueue(freeIDs, client_id, dataMutex);
                networking::closeFd(client_fd);
                return;
            }

            message_sent_success = matchmaking::sendHostTheGuestName(client_fd, guest.m_name);
            if (!message_sent_success)
            {
                std::print(stderr, "Error: message send unsucessful\n");
                critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, client_id, dataMutex, disconnectMutex);
                critical::invalidatePlayerOnceLobbyIsInvalid(players, lobbies, client_id, dataMutex);
                critical::addIDToQueue(freeIDs, client_id, dataMutex);
                networking::closeFd(client_fd);
                return;
            }

            bool oppWantsToPlay = true;
            while (oppWantsToPlay)
            {
                auto [hostPickedRed, disconnectedTmp1] = matchmaking::hostChoosesRed(client_fd);
                client_disconnected = disconnectedTmp1;
                if (client_disconnected)
                {
                    std::print(stderr, "Error: The host disconnected while choosing between red and blue.\n");
                    critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, client_id, dataMutex, disconnectMutex);
                    critical::invalidatePlayerOnceLobbyIsInvalid(players, lobbies, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd);
                    return;
                }
                std::print("Host chose the color: {}\n", (hostPickedRed) ? "red" : "blue");

                GameState gamestate = (hostPickedRed) ? GameState(client_player, guest) : GameState(guest, client_player);

                const bool gamestateAdded = critical::addGameStateToGameStates(gamestates, gamestate, client_id, dataMutex);
                if (!gamestateAdded)
                {
                    std::print(stderr, "Error: gamestate attempted to be added to gamestates while valid gamestate was still there\n");
                    critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, client_id, dataMutex, disconnectMutex);
                    critical::invalidatePlayerOnceLobbyIsInvalid(players, lobbies, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd);
                    return;
                }

                auto [wantToContinue, disconnectedTmp2, oppDisconnected] = play::playGame(hostPickedRed, client_id, client_fd, gamestates, gameMutexes);
                client_disconnected = disconnectedTmp2;
                hostWantsToPlay = wantToContinue;

                if (client_disconnected || !hostWantsToPlay)
                {
                    // TODO: either make sure you don't have the lock or you free it here
                    lobbies[client_id].m_hostPlayAgain = Lobby::PlayAgain::No;
                    critical::invalidateGamestateIfOtherPlayerDisconnected(gamestates, client_id, dataMutex, disconnectMutex);
                    critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, client_id, dataMutex, disconnectMutex);
                    critical::invalidatePlayerOnceLobbyIsInvalid(players, lobbies, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd);
                    return;
                }
                else // Host wants to play again
                {
                    lobbies[client_id].m_hostPlayAgain = Lobby::PlayAgain::Yes;
                    auto guestPlayAgain = Lobby::PlayAgain::Undecided;
                    while (!oppDisconnected) // wait for opp to decide if play again or no or disconnects
                    {
                        if (lobbies[client_id].m_guestPlayAgain != Lobby::PlayAgain::Undecided)
                        {
                            guestPlayAgain = lobbies[client_id].m_guestPlayAgain;
                            break;
                        }
                        oppDisconnected = lobbies[client_id].m_someoneDisconnected;
                        std::this_thread::sleep_for(std::chrono::seconds(2)); // check every few seconds
                    }
                    gamestates[client_id].m_isValid = false;

                    // Prepare to wait or play with opp again, depending on opp choice
                    if (oppDisconnected || guestPlayAgain == Lobby::PlayAgain::No)
                    {
                        oppWantsToPlay = false;
                        lobbies[client_id].m_hostPlayAgain = Lobby::PlayAgain::Undecided; // reset PlayAgain
                        lobbies[client_id].m_guestPlayAgain = Lobby::PlayAgain::Undecided;
                        // TODO: make sure that someoneDisconnected gets reset, but not before guest is finished cleaning up the lobby
                        // Maybe just wait in the loop above for the disconnect instead of PlayAgain::No.
                        // PlayAgain::No may honestly be unneeded. Just need Yes and Undecided.
                        lobbies[client_id].m_someoneDisconnected = false;
                        lobbies[client_id].m_guest = Player(); // Invalidate the lobby guest player
                    }
                    else // opp wants to play again
                    {
                        lobbies[client_id].m_guestPlayAgain = Lobby::PlayAgain::Undecided;
                    }
                    message_sent_success = matchmaking::sendClientOppPlayAgain(client_fd, oppWantsToPlay);
                    if (!message_sent_success)
                    {
                        std::print(stderr, "Error: message send unsucessful\n");
                        critical::invalidateGamestateIfOtherPlayerDisconnected(gamestates, client_id, dataMutex, disconnectMutex);
                        critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, client_id, dataMutex, disconnectMutex);
                        critical::invalidatePlayerOnceLobbyIsInvalid(players, lobbies, client_id, dataMutex);
                        critical::addIDToQueue(freeIDs, client_id, dataMutex);
                        networking::closeFd(client_fd);
                        return;
                    }
                }
            } // end opp wants to play loop
        } // end host wants to play loop
    }
    else // client wants to join existing lobby
    {
        bool guestWantsToPlay = true;
        while (guestWantsToPlay)
        {
            std::vector<Lobby> openLobbies = critical::getOpenLobbies(lobbies, dataMutex);
            message_sent_success = matchmaking::sendClientOpenLobbies(client_fd, openLobbies);
            if (!message_sent_success)
            {
                std::print(stderr, "Error: message send unsucessful\n");
                critical::invalidatePlayer(players, client_id, dataMutex);
                critical::addIDToQueue(freeIDs, client_id, dataMutex);
                networking::closeFd(client_fd);
                return;
            }

            // Receive the ID of the player guest wants to join.
            auto [hostID, disconnectedTmp1] = matchmaking::getClientLobbyChoice(client_fd);
            client_disconnected = disconnectedTmp1;
            if (client_disconnected)
            {
                critical::invalidatePlayer(players, client_id, dataMutex);
                critical::addIDToQueue(freeIDs, client_id, dataMutex);
                networking::closeFd(client_fd);
                return;
            }
            std::print("Guest made the choice: {}\n", hostID);

            // Check to make sure the lobby is still available.
            bool guestAdded = critical::addGuestToLobby(lobbies, hostID, client_player, dataMutex);
            message_sent_success = matchmaking::sendClientSuccessfulConnectionToLobby(client_fd, guestAdded);
            if (!message_sent_success)
            {
                std::print(stderr, "Error: message send unsucessful\n");
                if (guestAdded)
                {
                    critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, hostID, dataMutex, disconnectMutex);
                }
                critical::invalidatePlayer(players, client_id, dataMutex);
                critical::addIDToQueue(freeIDs, client_id, dataMutex);
                networking::closeFd(client_fd);
                return;
            }
            if (!guestAdded)
            {
                continue;
            }

            bool oppWantsToPlay = true;
            while (oppWantsToPlay)
            {
                // Block until host disconnects or chooses red or blue
                // TODO: make sure the guest doesn't get here before the host cleans up
                std::function<bool()> waitForHostColor = [&]
                {
                    bool hostLeft = lobbies[hostID].m_someoneDisconnected;
                    bool hostMadeChoice = gamestates[hostID].m_isValid;
                    bool keepWaiting = !hostLeft && !hostMadeChoice;
                    return !keepWaiting;
                };
                client_disconnected = matchmaking::blockUntilCondition(client_fd, waitForHostColor);
                if (client_disconnected)
                {
                    std::print(stderr, "Error: guest disconnected while waiting for host color.\n");
                    critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, hostID, dataMutex, disconnectMutex);
                    // NOTE: for now, I don't believe that I need to account for invalidating a player only when a lobby they were in closes.
                    // So we can just invalidate the player since they don't own the lobby.
                    critical::invalidatePlayer(players, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd);
                    return;
                }

                auto [hostPickedRed, hostDisconnected] = critical::hostPickedRed(gamestates, lobbies, hostID, dataMutex);
                char hostColor = (hostPickedRed) ? 'R' : 'B';
                hostColor = (hostDisconnected) ? 'D' : hostColor;
                std::print("Attempting to send the host color {}.\n", hostColor);
                message_sent_success = matchmaking::sendGuestTheHostColor(client_fd, hostColor);
                if (!message_sent_success)
                {
                    std::print(stderr, "Error: message send unsucessful\n");
                    critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, hostID, dataMutex, disconnectMutex);
                    // NOTE: for now, I don't believe that I need to account for invalidating a player only when a lobby they were in closes.
                    // So we can just invalidate the player since they don't own the lobby.
                    critical::invalidatePlayer(players, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd);
                    return;
                }
                if (hostDisconnected)
                {
                    critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, hostID, dataMutex, disconnectMutex);
                    break; // go back to searching for a lobby
                }

                auto [wantToContinue, disconnectedTmp2, oppDisconnected] = play::playGame(!hostPickedRed, hostID, client_fd, gamestates, gameMutexes);
                client_disconnected = disconnectedTmp2;
                guestWantsToPlay = wantToContinue;
                // TODO: fix the race condition or whatever happens here that causes guest to start and finish game without host.
                // It looks like it happens when the game ends (winner or stalemate) and the host says yes first, the guest then sees the end game state before it's cleared

                if (client_disconnected || !guestWantsToPlay)
                {
                    // TODO: either make sure you don't have the lock or you free it here
                    lobbies[hostID].m_guestPlayAgain = Lobby::PlayAgain::No;
                    critical::invalidateGamestateIfOtherPlayerDisconnected(gamestates, hostID, dataMutex, disconnectMutex);
                    critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, hostID, dataMutex, disconnectMutex);
                    critical::invalidatePlayer(players, client_id, dataMutex);
                    critical::addIDToQueue(freeIDs, client_id, dataMutex);
                    networking::closeFd(client_fd);
                    return;
                }
                else // Guest wants to play again
                {
                    lobbies[hostID].m_guestPlayAgain = Lobby::PlayAgain::Yes;
                    auto hostPlayAgain = Lobby::PlayAgain::Undecided;
                    while (!oppDisconnected) // wait for opp to decide if play again or no or disconnects
                    {
                        if (lobbies[hostID].m_hostPlayAgain != Lobby::PlayAgain::Undecided)
                        {
                            hostPlayAgain = lobbies[hostID].m_hostPlayAgain;
                            break;
                        }
                        oppDisconnected = lobbies[hostID].m_someoneDisconnected;
                        std::this_thread::sleep_for(std::chrono::seconds(2)); // check every few seconds
                    }

                    // Prepare to choose another lobby or play with opp again, depending on opp choice
                    if (oppDisconnected || hostPlayAgain == Lobby::PlayAgain::No)
                    {
                        oppWantsToPlay = false;
                    }
                    else // opp wants to play again
                    {
                        lobbies[hostID].m_hostPlayAgain = Lobby::PlayAgain::Undecided;
                    }
                    message_sent_success = matchmaking::sendClientOppPlayAgain(client_fd, oppWantsToPlay);
                    if (!message_sent_success)
                    {
                        std::print(stderr, "Error: message send unsucessful\n");
                        critical::invalidateGamestateIfOtherPlayerDisconnected(gamestates, hostID, dataMutex, disconnectMutex);
                        critical::invalidateLobbyIfOtherPlayerDisconnected(lobbies, hostID, dataMutex, disconnectMutex);
                        critical::invalidatePlayer(players, client_id, dataMutex);
                        critical::addIDToQueue(freeIDs, client_id, dataMutex);
                        networking::closeFd(client_fd);
                        return;
                    }
                }
            } // end opp wants to play loop
        } // end guest wants to play loop
    }

    // TODO: evaluate if this is needed
    critical::addIDToQueue(freeIDs, client_id, dataMutex);
    networking::closeFd(client_fd);
}

int main()
{

    int serv_fd = networking::initServer();
    std::mutex dataMutex;
    std::mutex disconnectMutex;

    // arraySize defined in settings.hpp
    std::array<Player, arraySize> players;
    std::array<GameState, arraySize> gameStates;
    std::array<Lobby, arraySize> lobbies;
    std::array<std::mutex, arraySize> gameMutexes;

    std::queue<std::uint8_t> freeIDs = std::queue<std::uint8_t>();
    initializeFreeIDs(freeIDs, arraySize);

    while (true)
    {
        int client_fd = networking::acceptConnection(serv_fd);
        if (client_fd == -1)
        {
            continue;
        }
        std::thread clientThread(manageClient, client_fd, std::ref(players), std::ref(gameStates), std::ref(lobbies), std::ref(freeIDs), std::ref(dataMutex), std::ref(disconnectMutex), std::ref(gameMutexes));
        clientThread.detach();
    }

    networking::closeFd(serv_fd);

    return 0;
}
