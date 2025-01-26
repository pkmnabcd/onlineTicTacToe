#include "ttt.hpp"
#include "interface.hpp"
#include "engine.hpp"
#include "ai.hpp"

#include <print>

void runGame()
{
    bool keepPlaying = true;
    while (keepPlaying)
    {
        interface::logo();
        std::uint8_t mode = interface::player_select();
        switch (mode)
        {
            case 0:
                engine::cpuVsCpu(ai::strategyOracle, ai::strategyOracle);
                break;
            case 1:
                engine::humanVsCpu(ai::strategyOracle);
                break;
            case 2:
                engine::cpuVsHuman(ai::strategyOracle);
                break;
            case 3:
                engine::humanVsHuman();
                break;
            case 'q':
                keepPlaying = false;
                break;
        }
    }
    std::print("Thanks for playing!\n");
}
