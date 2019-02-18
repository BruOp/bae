#include <bgfx/bgfx.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tinygltf/tiny_gltf.h>

#include "BasicGame.h"

int main(int argc, char* argv[])
{
    bae::BasicGame game{};
    game.start();
    return 0;
}
