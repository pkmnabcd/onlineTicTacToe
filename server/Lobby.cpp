#include "Lobby.hpp"

#include "Player.hpp"

Lobby::Lobby() :
    m_host(Player()),
    m_guest(Player()),
    m_isValid(false),
    m_waitingForHost(false),
    m_waitingForGuest(false),
    m_someoneDisconnected(false)
{
}

Lobby::Lobby(Player host) :
    m_host(host),
    m_guest(Player()),
    m_isValid(true),
    m_waitingForHost(false),
    m_waitingForGuest(false),
    m_someoneDisconnected(false)
{
}
