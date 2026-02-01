#include "matchmaking.hpp"
#include "networking.hpp"
#include "Player.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <tuple>

using StraightBoard = std::array<std::string, 9>;

const std::uint8_t NAME_LEN = 15; // NOTE: this includes terminating byte /0.


// TODO: see if I need to add 1 to each buffer len for \0
// TODO: I'll definitely need to add 1 to all the lens to account for \0.
// TODO: I might want to initialize the buffers with "" instead of nothing so that the memory is zeroed out. Or I need to check if disconnected before reading the buffer from a receiveAll

std::tuple<Player, bool, bool> matchmaking::getClientInfo(int client_fd, std::uint8_t client_id)
{
    bool disconnected = false;
    const int clientInfoBufferLen = NAME_LEN + 1;
    char clientInfoBuffer[clientInfoBufferLen];
    int numbytes = networking::receiveAll(client_fd, clientInfoBuffer, clientInfoBufferLen);

    // First char signifies whether client wants to host a game.
    // The other chars are the name the client picks
    // TODO: someday add limits to the names and check the current names to make sure it's unique
    std::string clientInfo(clientInfoBuffer);
    bool isHosting = clientInfo.at(0) == 'H';
    std::string client_name = clientInfo.substr(1);
    if (numbytes == -1 || numbytes == 0)
    {
        disconnected = true;
    }
    Player player = Player(client_name, client_id, client_fd);
    return std::make_tuple(player, disconnected, isHosting);
}

bool matchmaking::reportSuccessfulLobbyCreation(int client_fd)
{
    const int bufferLen = 8;
    char buffer[bufferLen] = "Success";
    int bytesSent = networking::sendAll(client_fd, buffer, bufferLen);

    return bytesSent == bufferLen;
}

bool matchmaking::sendHostTheGuestName(int client_fd, std::string guestName)
{
    assert(guestName.length() <= (NAME_LEN-1) && "Guest name not bigger than the maximum allowed name length");

    const int bufferLen = NAME_LEN;
    int bytesSent = networking::sendAll(client_fd, guestName.c_str(), bufferLen);
    return bytesSent == bufferLen;
}

std::tuple<bool, bool> matchmaking::hostChoosesRed(int client_fd)
{
    bool disconnected = false;
    const int chooseColorBufferLen = 1;
    char chooseColorBuffer[chooseColorBufferLen];
    int numbytes = networking::receiveAll(client_fd, chooseColorBuffer, chooseColorBufferLen);

    // First char signifies whether client wants to host a game.
    // The other chars are the name the client picks
    // TODO: someday add limits to the names and check the current names to make sure it's unique
    std::string colorChoice(chooseColorBuffer);
    bool choosesRed = colorChoice.at(0) == 'R';
    if (numbytes == -1 || numbytes == 0)
    {
        disconnected = true;
    }
    return std::make_tuple(choosesRed, disconnected);
}

bool matchmaking::sendGuestTheHostColor(int client_fd, bool hostChoseRed)
{
    const int bufferLen = 1;
    int bytesSent;
    if (hostChoseRed)
    {
        bytesSent = networking::sendAll(client_fd, "R", bufferLen);
    }
    else
    {
        bytesSent = networking::sendAll(client_fd, "B", bufferLen);
    }
    return bytesSent == bufferLen;
}

bool matchmaking::sendBoardState(int client_fd, StraightBoard board)
{
    const int bufferLen = 9;
    char buffer[bufferLen];

    for (std::uint8_t i = 0; i < board.size(); i++)
    {
        assert(board[i].length() == 1 && "Each board string has one char");
        buffer[i] = board[i].at(0);
    }
    int bytesSent;
    bytesSent = networking::sendAll(client_fd, buffer, bufferLen);
    return bytesSent == bufferLen;
}

bool isDigit(std::uint8_t ch)
{
    return ch >= 48 && ch <= 57;
}

std::tuple<std::uint8_t, bool> matchmaking::getClientMove(int client_fd)
{
    bool disconnected = false;
    const int chooseMoveBufferLen = 1;
    char chooseMoveBuffer[chooseMoveBufferLen];
    int numbytes = networking::receiveAll(client_fd, chooseMoveBuffer, chooseMoveBufferLen);

    // First char signifies whether client wants to host a game.
    // The other chars are the name the client picks
    // TODO: someday add limits to the names and check the current names to make sure it's unique
    std::uint8_t moveChoice;
    if (isDigit(chooseMoveBuffer[0]))
    {
        moveChoice = chooseMoveBuffer[0] - 48;
        if (moveChoice == 0)
        {
            disconnected = true;
        }
    }
    else
    {
        disconnected = true;
    }
    if (numbytes == -1 || numbytes == 0)
    {
        disconnected = true;
    }
    return std::make_tuple(moveChoice, disconnected);
}
