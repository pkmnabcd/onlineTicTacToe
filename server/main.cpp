#include "GameState.hpp"
#include "Lobby.hpp"
#include "Player.hpp"
#include "critical.hpp"
#include "matchmaking.hpp"
#include "networking.hpp"

#include <array>
#include <mutex>
#include <print>
#include <queue>
#include <thread>
#include <tuple>
#include <unistd.h>

void initializeFreeIDs(std::queue<std::uint8_t>& freeIDsQueue, std::size_t IDCount)
{
    for (std::size_t i = 0; i < IDCount; i++)
    {
        freeIDsQueue.push(i);
    }
}

void manageClient(int client_fd, std::array<Player, arraySize>& players, std::array<GameState, arraySize>& gameStates, std::array<Lobby, arraySize>& lobbies, std::queue<std::uint8_t>& freeIDs, std::mutex& dataMutex)
{
    bool client_disconnected = false;
    bool message_sent_success;
    gameStates[0].m_isValid = false; // TODO: remove this once I start using gameStates. This just gets rid of compiler warnings
    lobbies[0].m_isValid = false;    // TODO: remove this once I start using gameStates. This just gets rid of compiler warnings

    auto [client_id, noneAvailable] = critical::getAvailableID(freeIDs, dataMutex);
    if (noneAvailable)
    {
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
    }
    auto [client_player, disconnectedTmp0, isHosting] = matchmaking::getClientInfo(client_fd, client_id);
    client_disconnected = disconnectedTmp0;

    if (client_disconnected)
    {
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        networking::closeFd(client_fd);
    }
    const bool playerAdded = critical::addPlayerToPlayers(players, client_player, dataMutex);
    if (!playerAdded)
    {
        std::print(stderr, "Error: player attempted to be added to players while valid player was still there\n");
        critical::addIDToQueue(freeIDs, client_id, dataMutex);
        networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
    }

    // TODO: If Hosting, create a 'lobby' in which they can play games.
    // Lobby can only have the two players, but do this so games can be repeated and
    // players can swap being the red and blue player

    if (isHosting)
    {
        Lobby client_lobby = Lobby(client_player);
        message_sent_success = matchmaking::reportSuccessfulLobbyCreation(client_fd);
        if (!message_sent_success)
        {
            std::print(stderr, "Error: message send unsucessful\n");
            critical::addIDToQueue(freeIDs, client_id, dataMutex);
            networking::closeFd(client_fd); // TODO: Make sure that client knows why they got booted
        }
        // TODO:
        // add client's lobby to list of lobbies atomically
        // wait until the guest player's m_isValid is finally true (the thread for the guest player will have added their player to it)
        // send the guest name and do the logic to start the game (have host decide who is red, then start the exchange of inputs)
    }
    else // wants to join existing lobby
    {
        // TODO:
        // Read list of lobbies and send to client.
        // Expect client to either disconnect, try to join a lobby (which may or may not work if the lobby is now full or doesn't exist), or refresh the list.
    }

    // TODO: If joining, give a list of lobbies that need a second player.

    critical::addIDToQueue(freeIDs, client_id, dataMutex);
    networking::closeFd(client_fd);
}

int main()
{

    int serv_fd = networking::initServer();
    std::mutex dataMutex;

    const std::size_t arraySize = static_cast<std::size_t>(UINT8_MAX) + 1;
    std::array<Player, arraySize> players;
    std::array<GameState, arraySize> gameStates;
    std::array<Lobby, arraySize> lobbies;

    std::queue<std::uint8_t> freeIDs = std::queue<std::uint8_t>();
    initializeFreeIDs(freeIDs, arraySize);

    while (true)
    {
        int client_fd = networking::acceptConnection(serv_fd);
        if (client_fd == -1)
        {
            continue;
        }
        std::thread clientThread(manageClient, client_fd, std::ref(players), std::ref(gameStates), std::ref(lobbies), std::ref(freeIDs), std::ref(dataMutex));
        clientThread.detach();
    }

    networking::closeFd(serv_fd);

    return 0;
}
