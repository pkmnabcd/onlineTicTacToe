#include "util.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

util::Board util::makeBoard()
{
    return {{
        {"1", "2", "3"},
        {"4", "5", "6"},
        {"7", "8", "9"}
    }};
}

auto posToRowCol(std::uint8_t position)
{
    assert((position != 0 && position < 10) && "No out of bounds position");
    // The tuple has the values (row, col) positions on the board
    std::uint8_t cell = position -1;
    return std::make_tuple<std::uint8_t, std::uint8_t>(cell / 3, cell % 3);
}

std::uint8_t rowColToPos(std::uint8_t row, std::uint8_t col)
{
    std::uint8_t pos = row * 3 + col;
    return pos + 1;
}

std::optional<util::Board> util::place(util::Board board, std::uint8_t position, std::string player)
{
    assert((position != 0 && position < 10) && "No out of bounds position");
    assert((player == "X" || player == "O") && "Correct player characters");

    auto [row, col] = posToRowCol(position);

    if (board[row][col] != "X" && board[row][col] != "O")
    {
        Board newBoard;
        for (std::uint8_t r = 0; r < 3; r++)
        {
            for (std::uint8_t c = 0; c < 3; c++)
            {
                newBoard[r][c] = board[r][c];
            }
        }
        newBoard[row][col] = player;
        return std::optional<util::Board>(newBoard);
    }
    else
    {
        return {};
    }
}

std::uint8_t horizontalWinner(util::Board board)
{
    /*
     * Checks for and returns the winner in the horizontal direction
     * If no winner, 0 is returned
    */
    // Check first row
    if (board[0][0] == board[0][1] && board[0][0] == board[0][2])
    {
        return board[0][0][0];  // Assume cell has single char, X or O
    }
    else if (board[1][0] == board[1][1] && board[1][0] == board[1][2])
    {
        return board[1][0][0];
    }
    else if (board[2][0] == board[2][1] && board[2][0] == board[2][2])
    {
        return board[2][0][0];
    }
    else
    {
        return 0;
    }
}

std::uint8_t verticalWinner(util::Board board)
{
    /*
     * Checks for and returns the winner in the vertical direction
     * If no winner, 0 is returned
    */
    // Check first col
    if (board[0][0] == board[1][0] && board[0][0] == board[2][0])
    {
        return board[0][0][0];  // Assume cell has single char, X or O
    }
    else if (board[0][1] == board[1][1] && board[0][1] == board[2][1])
    {
        return board[0][1][0];
    }
    else if (board[0][2] == board[1][2] && board[0][2] == board[2][2])
    {
        return board[0][2][0];
    }
    else
    {
        return 0;
    }
}

std::uint8_t diagonalWinner(util::Board board)
{
    /*
     * Checks for and returns the winner in the diagonal direction
     * If no winner, 0 is returned
    */
    // Check first col
    if (board[0][0] == board[1][1] && board[0][0] == board[2][2])
    {
        return board[1][1][0];  // Assume cell has single char, X or O
    }
    else if (board[2][0] == board[1][1] && board[2][0] == board[0][2])
    {
        return board[1][1][0];
    }
    else
    {
        return 0;
    }
}

std::uint8_t util::winner(util::Board board)
{
    /*
     * Checks for and returns the winner
     * If no winner, 0 is returned
    */

    if (std::uint8_t horWinner = horizontalWinner(board); horWinner)
    {
        return horWinner;
    }
    else if (std::uint8_t verWinner = verticalWinner(board); verWinner)
    {
        return verWinner;
    }
    else if (std::uint8_t diaWinner = diagonalWinner(board); diaWinner)
    {
        return diaWinner;
    }
    else
    {
        return 0;
    }
}

// TODO: Finish the below functions
std::vector<std::uint8_t> util::openCells(util::Board board)
{
    std::vector<std::uint8_t> output;
    for (std::uint8_t row = 0; row < 3; row++)
    {
        for (std::uint8_t col = 0; col < 3; col++)
        {
            if (board[row][col] != "X" && board[row][col] != "O")
            {
                output.push_back(rowColToPos(row, col));
            }
        }
    }

    return output;
}

std::uint8_t util::firstOpenCell(util::Board board)
{
    /*
    * Returns 0 for no first open
    */
    std::vector<std::uint8_t> openCells = util::openCells(board);
    if (!openCells.empty())
    {
        return openCells[0];
    }
    else
    {
        return 0;
    }
}

bool util::full(util::Board board)
{
    return util::openCells(board).empty();
}

std::array<std::string, 9> util::get1dFrom2dBoard(util::Board board)
{
    std::array<std::string, 9> output;
    for (std::uint8_t row = 0; row < 3; row++)
    {
        for (std::uint8_t col = 0; col < 3; col++)
        {
            output[rowColToPos(row, col) - 1] = board[row][col];
        }
    }
    return output;
}
