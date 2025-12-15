#pragma once

#include "Player.hpp"

#include <array>
#include <cstdint>
#include <string>

using Board = std::array<std::array<std::string, 3>, 3>;
class GameState
{
  public:
    GameState(Board board, Player redPlayer, Player bluePlayer);
    GameState();

    Board m_board;
    const Player m_redPlayer;
    const Player m_bluePlayer;
    const bool m_isValid;
};
