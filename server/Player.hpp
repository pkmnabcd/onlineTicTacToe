#pragma once

#include <array>
#include <cstdint>
#include <string>

class Player
{
  public:
    Player(std::string name, std::uint8_t id, int socket);
    Player();

    const std::string m_name;
    const std::uint8_t m_id;
    const int m_socket;
    const bool m_isValidPlayer;
};
