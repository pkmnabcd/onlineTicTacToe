#include "GameState.hpp"

#include "Player.hpp"

#include <array>
#include <cstdint>
#include <string>

using Board = std::array<std::array<std::string, 3>, 3>;

GameState::GameState() :
    m_board(),
    m_redPlayer(),
    m_bluePlayer(),
    m_isValid(false)
{
}

GameState::GameState(Board board, Player redPlayer, Player bluePlayer) :
    m_board(board),
    m_redPlayer(redPlayer),
    m_bluePlayer(bluePlayer),
    m_isValid(true)
{
}
