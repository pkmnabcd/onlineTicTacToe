#pragma once

#include "Player.hpp"

#include <array>
#include <cstdint>
#include <string>

using StraightBoard = std::array<std::string, 9>;
class GameState
{
  public:
    GameState(Player redPlayer, Player bluePlayer);
    GameState();

    bool operator==(const GameState& rhs) const;

    bool isInitialState();

    StraightBoard m_board;
    Player m_redPlayer;
    Player m_bluePlayer;
    bool m_isValid;
};
