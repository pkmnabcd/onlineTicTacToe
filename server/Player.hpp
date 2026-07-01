#pragma once

#include <array>
#include <cstdint>
#include <string>

class Player
{
  public:
    Player(std::string name, std::uint8_t id);
    Player();

    std::string m_name;
    std::uint8_t m_id;
    bool m_isValid;
};
