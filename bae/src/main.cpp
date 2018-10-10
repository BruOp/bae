#include "Renderer.h"
#include <iostream>

int main()
{
    uint32_t width = 1280;
    uint32_t height = 720;

    bae::Renderer renderer{};
    renderer.init(width, height);

    while (renderer.update())
    {
        renderer.renderFrame();
    }

    return 0;
}
