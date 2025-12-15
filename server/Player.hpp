#pragma once

#include <array>
#include <cstdint>
#include <string>

class Player
{
  public:
    // TODO: change the socket type to whatever it's actually supposed to be!
    Player(std::string name, std::uint8_t id, std::string socket);
    Player();

    const std::string m_name;
    const std::uint8_t m_id;
    const std::string m_socket; // TODO: change this
    const bool m_isValidPlayer;
};
