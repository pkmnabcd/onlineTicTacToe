#pragma once

#include "Player.hpp"

#include <array>
#include <cstdint>

using StraightBoard = std::array<char, 9>;
class GameState
{
  public:
    GameState(Player redPlayer, Player bluePlayer);
    GameState();

    bool operator==(const GameState& rhs) const;

    bool isInitialState() const;

    StraightBoard m_board;
    Player m_redPlayer;
    Player m_bluePlayer;
    bool m_isValid;
    bool m_gameFinished;
};
