#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace util
{
    using Board = std::array<std::array<std::string, 3>, 3>;

    Board makeBoard();
    std::optional<Board> place(Board board, std::uint8_t position, std::string player);

    std::uint8_t winner(Board board);

    std::vector<std::uint8_t> openCells(Board board);
    std::uint8_t firstOpenCell(Board board);
    bool full(Board board);
    std::array<std::string, 9> get1dFrom2dBoard(Board board);

} // namespace util
