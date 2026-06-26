#include "matchmaking.hpp"

#include "networking.hpp"
#include "util.hpp"

#include <cassert>
#include <cstdint>
#include <format>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

const std::uint8_t NAME_LEN = 15; // NOTE: this includes terminating byte /0.

bool matchmaking::sendPlayerInfo(int serv_fd, bool hostGame, std::string username)
{
    const int playerInfoBufferLen = NAME_LEN + 1;
    char playerInfoBuffer[playerInfoBufferLen] = "";
    playerInfoBuffer[0] = (hostGame) ? 'H' : 'G';
    assert(username.length() < NAME_LEN && "Guest name should have length less than NAME_LEN");
    for (std::uint8_t i = 0; i < username.length(); i++)
    {
        playerInfoBuffer[i + 1] = username.at(i);
    }

    int bytesSent = networking::sendAll(serv_fd, playerInfoBuffer, playerInfoBufferLen);
    return bytesSent == playerInfoBufferLen;
}

std::tuple<std::string, bool> readStr(int serv_fd, char* buffer, const int bufferLen)
{
    bool disconnected = false;
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != bufferLen)
    {
        disconnected = true;
    }
    else
    {
        if (buffer[bufferLen - 1] != 0)
        {
            disconnected = true;
        }
    }
    std::string output(buffer);
    return std::make_tuple(output, disconnected);
}

std::tuple<std::uint8_t, bool> matchmaking::getYourID(int serv_fd)
{
    std::uint8_t id = 0;
    bool disconnected = false;

    const int bufferLen = 4;
    char buffer[bufferLen] = "";
    auto [idStr, disconnectedTmp0] = readStr(serv_fd, buffer, bufferLen);
    disconnected = disconnectedTmp0;
    if (!disconnected)
    {
        // Get the id from the string
        try
        {
            const unsigned long int idInt = std::stoul(idStr);
            id = static_cast<std::uint8_t>(idInt);
        }
        catch (std::invalid_argument const& ex)
        {
            disconnected = true;
        }
        catch (std::out_of_range const& ex)
        {
            disconnected = true;
        }
    }
    return std::make_tuple(id, disconnected);
}

std::tuple<bool, bool> matchmaking::getWaitStatus(int serv_fd)
{
    bool stillWaiting = true;
    bool disconnected = false;

    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != bufferLen)
    {
        disconnected = true;
    }
    else
    {
        if (buffer[0] == 'W')
        {
            stillWaiting = true;
        }
        else if (buffer[0] == 'R')
        {
            stillWaiting = false;
        }
        else
        {
            disconnected = true;
        }
    }
    return std::make_tuple(stillWaiting, disconnected);
}

bool matchmaking::sendPing(int serv_fd)
{
    const int bufferLen = 2;
    char buffer[bufferLen] = "Y";
    int bytesSent = networking::sendAll(serv_fd, buffer, bufferLen);
    return bytesSent == bufferLen;
}

std::tuple<std::string, bool> matchmaking::getGuestName(int serv_fd)
{
    std::string guestName = "";
    bool disconnected = false;

    const int bufferLen = NAME_LEN;
    char buffer[bufferLen] = "";
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != bufferLen)
    {
        disconnected = true;
    }
    else
    {
        if (buffer[NAME_LEN-1] != '\0')
        {
            disconnected = true;
        }
        else
        {
            guestName = std::string(buffer);
        }
    }
    return std::make_tuple(guestName, disconnected);
}

std::tuple<std::vector<std::tuple<std::string, std::uint8_t>>, bool> matchmaking::getOpenLobbies(int serv_fd)
{
    std::vector<std::tuple<std::string, std::uint8_t>> openLobbies;
    bool disconnected = false;

    const int bufferLen = 1;
    char buffer[bufferLen] = "";
    while(1)
    {
        std::string name = "";
        std::uint8_t id = 0;

        int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
        if (numbytes == -1 || numbytes == 0 || numbytes != bufferLen)
        {
            disconnected = true;
            break;
        }
        if (buffer[0] == 1)
        {
            // Get an id and name from the server
            char idBuffer[4] = "";
            auto [idStr, disconnectedTmp0] = readStr(serv_fd, idBuffer, 4);
            disconnected = disconnectedTmp0;
            if (disconnected)
            {
                break;
            }

            // TODO: change these to not magic numbers
            char nameBuffer[15] = "";
            auto [nameStr, disconnectedTmp1] = readStr(serv_fd, nameBuffer, 15);
            disconnected = disconnectedTmp1;
            if (disconnected)
            {
                break;
            }

            // Get the id from the string
            try
            {
                const unsigned long int idInt = std::stoul(idStr);
                id = static_cast<std::uint8_t>(idInt);
            }
            catch (std::invalid_argument const& ex)
            {
                disconnected = true;
                break;
            }
            catch (std::out_of_range const& ex)
            {
                disconnected = true;
                break;
            }
            name = nameStr;
            openLobbies.push_back(std::make_tuple(name, id));
        }
        else if (buffer[0] == 2)
        {
            break; // Done getting ids and names
        }
        else
        {
            disconnected = true;
            break;
        }
    }
    return std::make_tuple(openLobbies, disconnected);
}

bool matchmaking::sendLobbyChoice(int serv_fd, std::uint8_t hostID)
{
    std::string msg = std::format("{:3}", hostID);
    msg.push_back('\0');
    int bytesSent = networking::sendAll(serv_fd, msg.data(), msg.size()); // should be 4 bytes

    return bytesSent == static_cast<int>(msg.size());
}

std::tuple<bool, bool> matchmaking::getLobbyConnectionSuccessConfirmation(int serv_fd)
{
    bool connectionSuccess = false;
    bool disconnected = false;

    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != bufferLen)
    {
        disconnected = true;
    }
    else
    {
        if (buffer[0] == 'Y')
        {
            connectionSuccess = true;
        }
        else if (buffer[0] != 'N')
        {
            disconnected = true;
        }
    }
    return std::make_tuple(connectionSuccess, disconnected);
}

bool matchmaking::sendLobbyChoice(int serv_fd, bool choseRed)
{
    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    buffer[0] = (choseRed) ? 'R' : 'B';
    int bytesSent = networking::sendAll(serv_fd, buffer, bufferLen);

    return bytesSent == bufferLen;
}

bool matchmaking::blockAndPing(int serv_fd)
{
    /*
     * Block but receive pings from the server
    */
    bool disconnected = false;
    while (1)
    {
        auto [stillWaiting, disconnectedTmp1] = matchmaking::getWaitStatus(serv_fd);
        disconnected = disconnectedTmp1;
        if (disconnected || !stillWaiting)
        {
            break;
        }
    }
    return disconnected;
}

std::tuple<bool, bool, bool> matchmaking::getHostColor(int serv_fd)
{
    bool hostPickedRed = false;
    bool hostDisconnected = false;
    bool disconnected = false;

    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != bufferLen)
    {
        disconnected = true;
    }
    if (buffer[0] == 'R')
    {
        hostPickedRed = true;
    }
    else if (buffer[0] == 'D')
    {
        hostDisconnected = true;
    }
    else if (buffer[0] != 'B')
    {
        disconnected = true;
    }
    return std::make_tuple(hostPickedRed, hostDisconnected, disconnected);
}

std::tuple<bool, matchmaking::Winner, bool, bool> matchmaking::getGameStatus(int serv_fd)
{
    /*
     * Returns [continue?, winner, oppDisconnected, youDisconnected]
    */
    bool cont = true;
    matchmaking::Winner winner = matchmaking::Winner::Undecided;
    bool oppDisconnected = false;
    bool disconnected = false;

    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != bufferLen)
    {
        disconnected = true;
    }
    if (buffer[0] == 'X')
    {
        cont = false;
        winner = matchmaking::Winner::Red;
    }
    else if (buffer[0] == 'O')
    {
        cont = false;
        winner = matchmaking::Winner::Blue;
    }
    else if (buffer[0] == 'S')
    {
        cont = false;
        winner = matchmaking::Winner::Stalemate;
    }
    else if (buffer[0] == 'D')
    {
        cont = false;
        oppDisconnected = true;
    }
    else if (buffer[0] != 'C')
    {
        cont = false;
        disconnected = true;
    }
    return std::make_tuple(cont, winner, oppDisconnected, disconnected);
}

Board getBoardFromStraightBoard(StraightBoard straightBoard)
{
    Board board;
    std::uint8_t s_index = 0;
    for (std::uint8_t i = 0; i < 3; i++)
    {
        for (std::uint8_t j = 0; j < 3; j++)
        {
            board[i][j] = straightBoard[s_index];
            s_index++;
        }
    }
    return board;
}

bool isGoodSpace(char c)
{
    return c == 'X' || c == 'O' || (util::isDigit(c) && c != '0');
}

std::tuple<Board, bool> matchmaking::getBoardState(int serv_fd)
{
    Board board;
    bool disconnected = false;

    const int bufferLen = 10;
    char buffer[bufferLen] = "";
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != bufferLen)
    {
        disconnected = true;
    }
    else
    {
        StraightBoard sBoard;
        for (std::uint8_t i = 0; i < bufferLen-1; i++)
        {
            if (!isGoodSpace(buffer[i]))
            {
                disconnected = true;
                break;
            }
            sBoard[i].at(0) = buffer[i];
        }
        board = getBoardFromStraightBoard(sBoard);
    }
    return std::make_tuple(board, disconnected);
}

bool sendMove(int serv_fd, char move)
{
    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    buffer[0] = move;
    int bytesSent = networking::sendAll(serv_fd, buffer, bufferLen);

    return bytesSent == bufferLen;
}

bool matchmaking::sendPlayAgain(int serv_fd, bool playAgain)
{
    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    buffer[0] = (playAgain) ? 'Y' : 'N';
    int bytesSent = networking::sendAll(serv_fd, buffer, bufferLen);

    return bytesSent == bufferLen;
}

std::tuple<bool, bool> matchmaking::getOppPlayAgain(int serv_fd)
{
    bool oppPlayAgain = false;
    bool disconnected = false;

    const int bufferLen = 2;
    char buffer[bufferLen] = "";
    int numbytes = networking::receiveAll(serv_fd, buffer, bufferLen);
    if (numbytes == -1 || numbytes == 0 || numbytes != bufferLen)
    {
        disconnected = true;
    }
    else
    {
        if (buffer[0] == 'Y')
        {
            oppPlayAgain = true;
        }
        else if (buffer[0] != 'N')
        {
            disconnected = true;
        }
    }
    return std::make_tuple(oppPlayAgain, disconnected);
}
