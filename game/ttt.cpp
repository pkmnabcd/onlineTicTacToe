#include "ttt.hpp"
#include "interface.hpp"

// Temp
#include "engine.hpp"
#include "util.hpp"

#include <print>

void runGame()
{
    // Temp code
    {
        engine::humanTurn(util::makeBoard(), "X");
    }
    bool keepPlaying = true;
    while (keepPlaying)
    {
        interface::logo();
        std::int8_t mode = interface::player_select();
        switch (mode)
        {
            case 0:
                // cpu vs cpu
                break;
            case 1:
                // human vs cpu
                break;
            case 2:
                // cpu vs human
                break;
            case 3:
                // human vs human
                break;
            case 'q':
                keepPlaying = false;
                break;
        }
    }
    std::print("Thanks for playing!\n");
}
