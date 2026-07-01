#include "Player.hpp"

#include <array>
#include <cstdint>
#include <string>

Player::Player() :
    m_name(""),
    m_id(0),
    m_isValid(false)
{
}

Player::Player(std::string name, std::uint8_t id) :
    m_name(name),
    m_id(id),
    m_isValid(true)
{
}
