#pragma once

#include "Player.hpp"

class Lobby
{
  public:
    Lobby(Player host);
    Lobby();

    enum class PlayAgain
    {
        Undecided,
        Yes,
        No
    };

    Player m_host;
    Player m_guest;
    PlayAgain m_hostPlayAgain;
    PlayAgain m_guestPlayAgain;
    bool m_isValid;
    bool m_waitingForHost;
    bool m_waitingForGuest;
    bool m_someoneDisconnected;
};
