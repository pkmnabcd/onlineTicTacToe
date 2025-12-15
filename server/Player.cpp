#include "Player.hpp"

#include <array>
#include <cstdint>
#include <string>

Player::Player() :
    m_name(""),
    m_id(0),
    m_socket(""),
    m_isValidPlayer(false)
{
}

Player::Player(std::string name, std::uint8_t id, std::string socket) :
    m_name(name),
    m_id(id),
    m_socket(socket),
    m_isValidPlayer(true)
{
}
