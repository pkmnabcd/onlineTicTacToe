#pragma once

#include "Player.hpp"

class Lobby
{
  public:
    Lobby(Player host);
    Lobby();

    Player m_host;
    Player m_guest;
    bool m_isValid;
};
