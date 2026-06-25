#include "play.hpp"

#include "matchmaking.hpp"

#include <tuple>


std::tuple<bool, bool, bool> play::playGame(int serv_fd, bool isRed)
{
    /*
     * Returns [wantsToPlayAgain: bool, disconnected: bool, oppDisconnected: bool]
     */
    // TODO: Process for this function:
    // 1. get the game status
    // 2. get the board state
    // 3. get move from user and send to server
    // 4. get game status again
    // 5. Repeat until status other than 'C'
    // 6. Decide if you want to play again
    return std::make_tuple(true, true, true);
}
