#include "GameState.hpp"

#include "Player.hpp"

#include <array>
#include <cstdint>
#include <string>

using StraightBoard = std::array<std::string, 9>;

GameState::GameState() :
    m_board(),
    m_redPlayer(),
    m_bluePlayer(),
    m_isValid(false)
{
}

GameState::GameState(Player redPlayer, Player bluePlayer) :
    m_board({"1", "2", "3", "4", "5", "6", "7", "8", "9"}),
    m_redPlayer(redPlayer),
    m_bluePlayer(bluePlayer),
    m_isValid(true)
{
}
