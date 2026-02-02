#pragma once

#include <array>
#include <cstdint>
#include <string>

class Player
{
  public:
    Player(std::string name, std::uint8_t id, int socket);
    Player();

    std::string m_name;
    std::uint8_t m_id;
    int m_socket;
    bool m_isValid;
};
