
#include "Game.h"


int main()
{
    SDL_SetMainReady();

    Game game;

    game.Init();
    game.Run();

    return 0;
}
