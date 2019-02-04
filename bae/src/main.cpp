#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "BasicGame.h"

int main(int argc, char* argv[])
{
    bae::BasicGame game{};
    game.start();
    return 0;
}
