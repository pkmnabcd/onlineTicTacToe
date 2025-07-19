#pragma once

using Board = std::array<std::array<std::string, 3>, 3>;
class GameState
{
  public:
  private:
    Board board;
    std::uint8_t redID;
    std::uint8_t blueID;
}
