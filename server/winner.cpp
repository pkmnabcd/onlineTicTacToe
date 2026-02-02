#include "winner.hpp"

#include <array>
#include <cstdint>
#include <string>

using StraightBoard = std::array<std::string, 9>;


std::uint8_t horizontalWinner(StraightBoard board)
{
    /*
     * Checks for and returns the winner in the horizontal direction
     * If no winner, 0 is returned
    */
    // Check first row
    if (board[0] == board[1] && board[0] == board[2])
    {
        return board[0][0];  // Assume cell has single char, X or O
    }
    else if (board[3] == board[4] && board[3] == board[5])
    {
        return board[3][0];
    }
    else if (board[6] == board[7] && board[6] == board[8])
    {
        return board[6][0];
    }
    else
    {
        return 0;
    }
}

std::uint8_t verticalWinner(StraightBoard board)
{
    /*
     * Checks for and returns the winner in the vertical direction
     * If no winner, 0 is returned
    */
    // Check first col
    if (board[0] == board[3] && board[0] == board[6])
    {
        return board[0][0];  // Assume cell has single char, X or O
    }
    else if (board[1] == board[4] && board[1] == board[7])
    {
        return board[1][0];
    }
    else if (board[2] == board[5] && board[2] == board[6])
    {
        return board[2][0];
    }
    else
    {
        return 0;
    }
}

std::uint8_t diagonalWinner(StraightBoard board)
{
    /*
     * Checks for and returns the winner in the diagonal direction
     * If no winner, 0 is returned
    */
    // Check first col
    if (board[0] == board[4] && board[0] == board[8])
    {
        return board[4][0];  // Assume cell has single char, X or O
    }
    else if (board[2] == board[4] && board[2] == board[6])
    {
        return board[4][0];
    }
    else
    {
        return 0;
    }
}

std::uint8_t winner::winner(StraightBoard board)
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
