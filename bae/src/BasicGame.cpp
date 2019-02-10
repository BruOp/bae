#include "BasicGame.h"

namespace bae {

BasicGame::BasicGame() noexcept
    : renderer{}
    , registry{}
    , startOffset{ static_cast<uint64_t>(bx::getHPCounter()) }
    , lastTime{ getTime(startOffset) }
{
    uint32_t width = 1280;
    uint32_t height = 720;

    windowContext.initSDL();

    pWindow = std::make_unique<bae::Window>(width, height);

    renderer.init(pWindow.get());

    camera = Camera{
        Position{ 0.0f, 1.0f, 5.0f },
        Direction{ 0.0f, 0.0f, -1.0f },
        width,
        height,
        60.0f
    };
    cameraControls = FPSControls{ camera };

    auto entity = registry.create();

    // Create our mesh
    registry.assign<Position>(entity, 2.0f, 0.0f, 0.0f);
    registry.assign<Geometry>(entity, renderer.geoRegistry.get("materialSphere"));
    registry.assign<Materials::Basic>(entity, glm::vec4(0.9f, 0.1f, 0.1f, 1.0f));

    entity = registry.create();

    // Create our mesh
    registry.assign<Position>(entity, -4.0f, -1.0f, 0.0f);
    registry.assign<Geometry>(entity, renderer.geoRegistry.get("bunny"));
    registry.assign<Materials::Basic>(entity, glm::vec4(0.97f, 0.96f, 0.91f, 1.0f));

    auto light = registry.create();
    registry.assign<Position>(light, 15.0f, 0.0f, 0.0f);
    registry.assign<PointLightEmitter>(light, glm::vec3{ 1.0f, 1.0f, 0.2f }, 100.0f);

    auto light2 = registry.create();
    registry.assign<Position>(light2, -10.0f, 10.0f, 0.0f);
    registry.assign<PointLightEmitter>(light2, glm::vec3{ 1.0f, 1.0f, 1.0f }, 100.0f);
}

void BasicGame::start()
{
    while (update()) {
    }
}

bool BasicGame::update()
{
    float _time = getTime(startOffset);
    float dt = _time - lastTime;

    auto res = eventQueue.pump();
    auto eventhandleResult = windowInputHandler.handleEvents(eventQueue);
    if (eventhandleResult == EventHandleResult::EVENT_RESULT_SHUTDOWN) {
        return false;
    }
    auto inputHandleResult = cameraControls.handleEvents(eventQueue);
    if (inputHandleResult == EventHandleResult::EVENT_RESULT_SHUTDOWN) {
        return false;
    }
    cameraControls.update(dt);
    renderer.renderFrame(dt, camera, registry);
    lastTime = _time;
    return true;
}
}
