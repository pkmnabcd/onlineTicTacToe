#include "GameState.hpp"

#include <array>
#include <cstdint>
#include <string>

using Board = std::array<std::array<std::string, 3>, 3>;

GameState::GameState(Board board, std::uint8_t redID, std::uint8_t blueID, bool setIsRedTurn) :
    m_board = board,
    m_redID = redID,
    m_redID = redID,
    m_isRedTurn = isRedTurn
{
}
