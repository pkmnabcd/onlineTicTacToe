#include "Lobby.hpp"

#include "Player.hpp"

Lobby::Lobby() :
    m_host(Player()),
    m_guest(Player()),
    m_isValid(false)
{
}

Lobby::Lobby(Player host) :
    m_host(host),
    m_guest(Player()),
    m_isValid(true)
{
}
