#include <iostream>
#include "./Game.h"

int main(int argc, char *args[]) {
    Game *game = new Game();

    game->Initialize(true);

    while (game->IsRunning()) {
        game->ProcessInput();
        game->Update();
        game->Render();
    }

    game->Destroy();

    return 0;
}
