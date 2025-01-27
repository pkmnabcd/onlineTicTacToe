#include "../game/interface.hpp"
#include "../game/ttt.hpp"

int main()
{
    bool keepPlaying = true;
    while (keepPlaying)
    {
        interface::logo();
        std::uint8_t mode = interface::selectGameMode();
        switch (mode)
        {
            case 0:
                runGame();
                break;
            case 'q':
                keepPlaying = false;
                break;
        }
    }

    return 0;
}
