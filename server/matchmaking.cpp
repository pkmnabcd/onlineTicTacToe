#include "matchmaking.hpp"
#include "networking.hpp"
#include "settings.hpp"
#include "Lobby.hpp"
#include "Player.hpp"

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

using StraightBoard = std::array<char, 9>;
const std::uint8_t NAME_LEN = 15; // NOTE: this includes terminating byte /0.

// TODO: change the paradigm so that I don't just rely on the order of
// packages being sent/received to function. Ex: have tags at the start
// of each send/recv message that say the purpose of the packet. Ex 0x01
// for pings, 0x02 for moves, etc

bool nameIsValid(std::string name)
{
    bool isValid = true;
    if (name.size() > 14 || name.size() == 0)
    {
        isValid = false;
    }
    for (std::uint8_t i = 0; i < name.size(); i++)
    {
        char c = name.at(i);
        if (c < 33 || c > 126)
        {
            isValid = false;
        }
    }
    return isValid;
}

std::tuple<Player, bool, bool> matchmaking::getClientInfo(SocketType client_fd, std::uint8_t client_id)
{
    Player player = Player();
    bool disconnected = false;
    bool isHosting = false;

    const int clientInfoBufferLen = NAME_LEN + 1;
    char clientInfoBuffer[clientInfoBufferLen] = "";
    int numbytes = networking::receiveAll(client_fd, clientInfoBuffer, clientInfoBufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != clientInfoBufferLen)
    {
        disconnected = true;
    }
    else
    {
        if (clientInfoBuffer[clientInfoBufferLen - 1] != '\0')
        {
            disconnected = true; // name should end with \0
        }
        else
        {
            // First char signifies whether client wants to host a game.
            // The other chars are the name the client picks
            // TODO: Someday check the current names to make sure it's unique
            std::string clientInfo(clientInfoBuffer);
            std::string client_name = clientInfo.substr(1);
            if (!nameIsValid(client_name))
            {
                disconnected = true;
            }
            else
            {
                player = Player(client_name, client_id);
                isHosting = clientInfo.at(0) == 'H';
            }
        }
    }
    return std::make_tuple(player, disconnected, isHosting);
}

bool matchmaking::sendClientID(SocketType client_fd, std::uint8_t client_id)
{
    std::string msg = "";
    msg.append(std::format("{:3}", client_id));
    msg.push_back('\0');
    int bytesSent = networking::sendAll(client_fd, msg.data(), static_cast<int>(msg.size())); // should be 4 bytes

    return bytesSent == static_cast<int>(msg.size());
}

bool matchmaking::sendHostTheGuestName(SocketType client_fd, std::string guestName)
{
    assert(guestName.length() <= (NAME_LEN-1) && "Guest name not bigger than the maximum allowed name length");

    const int bufferLen = NAME_LEN;
    int bytesSent = networking::sendAll(client_fd, guestName.c_str(), bufferLen);
    return bytesSent == bufferLen;
}

std::tuple<bool, bool> matchmaking::hostChoosesRed(SocketType client_fd)
{
    bool choosesRed = false;
    bool disconnected = false;

    const int chooseColorBufferLen = 2;
    char chooseColorBuffer[chooseColorBufferLen] = "";
    int numbytes = networking::receiveAll(client_fd, chooseColorBuffer, chooseColorBufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != chooseColorBufferLen)
    {
        disconnected = true;
    }
    else
    {
        std::string colorChoice(chooseColorBuffer);
        if (colorChoice.at(0) == 'R')
        {
            choosesRed = true;
        }
        else if (colorChoice.at(0) != 'B')
        {
            disconnected = true;
        }
    }
    return std::make_tuple(choosesRed, disconnected);
}

bool matchmaking::sendGuestTheHostColor(SocketType client_fd, char hostColor)
{
    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    buffer[0] = hostColor;
    int bytesSent;
    bytesSent = networking::sendAll(client_fd, buffer, bufferLen);
    return bytesSent == bufferLen;
}

bool matchmaking::sendBoardState(SocketType client_fd, StraightBoard board)
{
    // NOTE: 9 for board, +1 for null terminal
    const int bufferLen = 10;
    char buffer[bufferLen] = "";

    for (std::uint8_t i = 0; i < board.size(); i++)
    {
        buffer[i] = board[i];
    }
    int bytesSent;
    bytesSent = networking::sendAll(client_fd, buffer, bufferLen);
    return bytesSent == bufferLen;
}

bool isDigit(std::uint8_t ch)
{
    return ch >= 48 && ch <= 57;
}

std::tuple<std::uint8_t, bool, bool> matchmaking::getClientMove(SocketType client_fd)
{
    std::uint8_t moveChoice = 0;
    bool clientQuit = false;
    bool disconnected = false;

    const int chooseMoveBufferLen = 2;
    char chooseMoveBuffer[chooseMoveBufferLen] = "";
    int numbytes = networking::receiveAll(client_fd, chooseMoveBuffer, chooseMoveBufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != chooseMoveBufferLen)
    {
        disconnected = true;
    }
    else
    {
        if (isDigit(chooseMoveBuffer[0])) // Should be 1-9. 0 is invalid
        {
            moveChoice = chooseMoveBuffer[0] - 48;
            if (moveChoice == 0)
            {
                disconnected = true;
            }
        }
        else
        {
            if (chooseMoveBuffer[0] == 'q')
            {
                clientQuit = true;
            }
            disconnected = true;
        }
    }
    return std::make_tuple(moveChoice, clientQuit, disconnected);
}

bool matchmaking::sendClientGameStatus(SocketType client_fd, char winnerOrContOrOppDiscon)
{
    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    // NOTE: 'C' for continue, 'X' for red wins, 'O' for blue wins,
    // 'S' for stalemate, 'D' for opponent disconnected
    buffer[0] = winnerOrContOrOppDiscon;
    int bytesSent;
    bytesSent = networking::sendAll(client_fd, buffer, bufferLen);
    return bytesSent == bufferLen;
}

std::tuple<bool, bool> matchmaking::getClientPlayAgain(SocketType client_fd)
{
    bool playAgain = false;
    bool disconnected = false;

    const int playAgainBufferLen = 2;
    char playAgainBuffer[playAgainBufferLen] = "";
    int numbytes = networking::receiveAll(client_fd, playAgainBuffer, playAgainBufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != playAgainBufferLen)
    {
        disconnected = true;
    }
    else
    {
        if (playAgainBuffer[0] == 'Y')
        {
            playAgain = true;
        }
        else if (playAgainBuffer[0] == 'N')
        {
            playAgain = false;
        }
        else
        {
            disconnected = true;
        }
    }
    return std::make_tuple(playAgain, disconnected);
}

bool matchmaking::sendClientOppPlayAgain(SocketType client_fd, bool oppPlayAgain)
{
    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    buffer[0] = (oppPlayAgain) ? 'Y' : 'N';
    int bytesSent;
    bytesSent = networking::sendAll(client_fd, buffer, bufferLen);
    return bytesSent == bufferLen;
}

bool matchmaking::sendClientOpenLobbies(SocketType client_fd, std::vector<Lobby> openLobbies)
{
    std::string msg = "";
    for (Lobby& lobby : openLobbies)
    {
        // NOTE: 3 chars for id, NAME_LEN-1 = 14 for the name and also
        // add \x01 for a sign to expect the lobby and also include the \0 s
        msg.push_back('\x01');
        msg.append(std::format("{:3}", lobby.m_host.m_id));
        msg.push_back('\0');
        msg.append(std::format("{:14}", lobby.m_host.m_name));
        msg.push_back('\0');
    }
    msg.push_back('\x02'); // sign to stop expecting lobbies

    const int bufferLen = static_cast<int>(msg.size());
    int bytesSent = networking::sendAll(client_fd, msg.data(), bufferLen);
    return bytesSent == bufferLen;
}

std::tuple<std::uint8_t, bool> matchmaking::getClientLobbyChoice(SocketType client_fd)
{
    std::uint8_t hostID = 0;
    bool disconnected = false;
    const int lobbyBufferLen = 4; // 3 for id in base-10, 1 for \0
    char lobbyBuffer[lobbyBufferLen] = "";
    int numbytes = networking::receiveAll(client_fd, lobbyBuffer, lobbyBufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != lobbyBufferLen)
    {
        disconnected = true;
    }
    else
    {
        // Check for valid numbers
        for (short i = 0; i < lobbyBufferLen-1; i++) // NOTE: ignore \0 at end
        {
            if (!isDigit(lobbyBuffer[i]) && lobbyBuffer[i] != ' ')
            {
                disconnected = true;
                break;
            }
        }
        if (disconnected)
        {
            return std::make_tuple(hostID, disconnected);
        }
        unsigned long hostID_tmp = std::stoul(std::string(lobbyBuffer));
        if (hostID_tmp > arraySize - 1)
        {
            disconnected = true;
        }
        else
        {
            hostID = static_cast<std::uint8_t>(hostID_tmp);
        }
    }

    return std::make_tuple(hostID, disconnected);
}

bool matchmaking::sendClientSuccessfulConnectionToLobby(SocketType client_fd, bool successfulConnection)
{
    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    buffer[0] = (successfulConnection) ? 'Y' : 'N';
    int bytesSent;
    bytesSent = networking::sendAll(client_fd, buffer, bufferLen);
    return bytesSent == bufferLen;
}

bool matchmaking::getClientCheckIn(SocketType client_fd)
{
    bool disconnected = false;
    const int lobbyBufferLen = 2;
    char lobbyBuffer[lobbyBufferLen] = "";
    int numbytes = networking::receiveAll(client_fd, lobbyBuffer, lobbyBufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != lobbyBufferLen)
    {
        disconnected = true;
    }
    else
    {
        if (lobbyBuffer[0] != 'Y' || lobbyBuffer[1] != '\0')
        {
            disconnected = true;
        }
    }
    return disconnected;
}

bool matchmaking::sendCheckIn(SocketType client_fd, bool stillWaiting)
{
    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    buffer[0] = (stillWaiting) ? 'W' : 'R';
    int bytesSent;
    bytesSent = networking::sendAll(client_fd, buffer, bufferLen);
    return bytesSent == bufferLen;
}

bool matchmaking::blockUntilCondition(SocketType client_fd, std::function<bool()> condition)
{
    bool disconnected = false;
    while (true)
    {
        bool message_sent_success = matchmaking::sendCheckIn(client_fd, true); // Still waiting
        if (!message_sent_success)
        {
            disconnected = true;
            break;
        }

        if (condition())
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2)); // check every few seconds
    }

    if (!disconnected)
    {
        bool message_sent_success = matchmaking::sendCheckIn(client_fd, false); // Done waiting
        if (!message_sent_success)
        {
            disconnected = true;
        }
    }
    return disconnected;
}
