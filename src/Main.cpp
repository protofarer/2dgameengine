#include "Game/Game.h"

int main(int argc, char* argv[]) {
    Game game;  // c++ constructs w/o "new", stores on stack (instead of dyn alloc/heap), destroyed at scope end.

    game.Initialize();
    game.Run();
    game.Destroy();

    return 0;
}