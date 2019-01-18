#include "BasicGame.h"

namespace bae {

BasicGame::BasicGame()
    : renderer{}
    , registry{}
    , startOffset{ bx::getHPCounter() }
    , lastTime{ getTime(startOffset) }
{
    uint32_t width = 1280;
    uint32_t height = 720;

    renderer.init(width, height);

    auto entity = registry.create();

    // Create our mesh
    registry.assign<Position>(entity, 0.0f, 0.0f, 0.0f);
    registry.assign<Geometry>(entity, renderer.geoRegistry.get("bunny"));
    registry.assign<Materials::Basic>(entity, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));

    auto light = registry.create();
    registry.assign<Position>(light, 0.0f, 5.0f, 0.0f);
    registry.assign<PointLightEmitter>(light, glm::vec3{ 1.0f, 0.0f, 1.0f }, 100.0f);

    auto light2 = registry.create();
    registry.assign<Position>(light2, -10.0f, 10.0f, 0.0f);
    registry.assign<PointLightEmitter>(light2, glm::vec3{ 1.0f, 0.0f, 0.0f }, 100.0f);
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
    renderer.renderFrame(dt, registry);
    lastTime = _time;
    return true;
}
}
