#include <bx/bx.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
// STB_IMAGE_IMPLEMENTATION is defined through bx already!
#define TINYGLTF_NO_STB_IMAGE_WRITE 
#include <tinygltf/tiny_gltf.h>

#include "BasicGame.h"

int main(int argc, char* argv[])
{
	bae::BasicGame game{};
	game.start();
    return 0;
}
