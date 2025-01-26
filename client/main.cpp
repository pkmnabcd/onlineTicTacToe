#include "../game/interface.hpp"
#include "../game/ttt.hpp"

int main()
{
    bool keepPlaying = true;
    while (keepPlaying)
    {
        interface::logo();
        interface::selectGameMode();
        keepPlaying = false;
    }
    runGame();

    return 0;
}
