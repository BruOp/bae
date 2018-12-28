#include "BasicGame.h"

namespace bae {

BasicGame::BasicGame()
    : renderer{}
    , startOffset{ bx::getHPCounter() }
    , lastTime{ getTime(startOffset) }
{
    uint32_t width = 1280;
    uint32_t height = 720;

    renderer.init(width, height);
}

BasicGame::~BasicGame() {}

void BasicGame::start()
{
    while (update()) {
    }
}

bool BasicGame::update()
{
    float _time = getTime(startOffset);
    float dt = _time - lastTime;
    // TODO: Remove all the logic from renderer update. There's no reason for it to own all this stuff
    if (!renderer.update(dt)) {
        return false;
    };
    renderer.renderFrame(dt);
    lastTime = _time;
    return true;
}
}
