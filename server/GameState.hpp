#pragma once

#include <array>
#include <cstdint>
#include <string>

using Board = std::array<std::array<std::string, 3>, 3>;
class GameState
{
  public:
    void GameState(Board board, std::uint8_t redID, std::uint8_t blueID, bool isRedTurn);

    Board getBoard() { return m_board }
    std::uint8_t getRedID() { return m_redID }
    std::uint8_t getBlueID() { return m_blueID }
    bool getIsRedTurn() { return m_isRedTurn }

    void setBoard(Board board) { m_board = board }
    void setRedID(std::uint8_t redID) { m_redID = redID }
    void setBlueID(std::uint8_t blueID) { m_blueID = blueID }
    void setIsRedTurn(bool isRedTurn) { m_isRedTurn = isRedTurn }

  private:
    Board m_board;
    std::uint8_t m_redID;
    std::uint8_t m_blueID;
    bool m_isRedTurn;
}
