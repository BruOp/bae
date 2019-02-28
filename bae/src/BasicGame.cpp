#include "BasicGame.h"
#include "GltfModelLoader.h"

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
    
    std::string gltfDir = GLTF_DIR;
    //std::string modelPath = gltfDir + "Cube/glTF/Cube.gltf";
    std::string modelPath = gltfDir + "FlightHelmet/glTF/FlightHelmet.gltf";
    GltfModelLoader modelLoader{ registry, renderer.geoRegistry, renderer.textureManager };
    std::vector<uint32_t> entities = modelLoader.loadModel(modelPath);
    
    auto light = registry.create();
    registry.assign<Position>(light, 15.0f, 0.0f, 0.0f);
    registry.assign<PointLightEmitter>(light, glm::vec3{ 1.0f, 1.0f, 0.2f }, 50.0f);

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
    renderer.renderFrame(dt, _time, camera, registry);
    lastTime = _time;
    return true;
}
}
