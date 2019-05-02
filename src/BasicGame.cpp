#include "BasicGame.h"
#include "Cube.cpp"
#include "GltfModelLoader.h"

namespace bae
{
BasicGame::BasicGame() noexcept : renderer{},
                                  registry{},
                                  startOffset{static_cast<uint64_t>(bx::getHPCounter())},
                                  lastTime{getTime(startOffset)}
{
    uint32_t width = 1280;
    uint32_t height = 720;

    windowContext.initSDL();

    pWindow = std::make_unique<bae::Window>(width, height);

    renderer.init(pWindow.get());

    camera = Camera{Position{-0.0f, 0.0f, 1.0f}, Direction{0.0f, -2.f, -1.f}, width, height, 60.0f};
    cameraControls = FPSControls{camera};

    std::string gltfDir = GLTF_DIR;
    std::string modelPath = gltfDir + "FlightHelmet/glTF/";
    GltfModelLoader modelLoader{registry, renderer.geoRegistry, renderer.textureManager};
    std::vector<uint32_t> entities = modelLoader.loadModel(modelPath, "FlightHelmet");

    auto cubeGeometry = renderer.geoRegistry.create<PosColorVertex>("cube", cubeVertices, cubeIndices);
    glm::vec3 lightMeshScalingFactor{0.05f};

    auto light = registry.create();
    Position pos{5.0f, 2.0f, 0.0f};
    glm::vec3 color{1.0f, 0.5f, 0.2f};
    registry.assign<Position>(light, pos);
    registry.assign<PointLightEmitter>(light, color, 10.0f);
    // This allows us to "see" the light as a cube in the scene.
    registry.assign<Geometry>(light, cubeGeometry);
    registry.assign<Materials::Basic>(light, glm::vec4{color.x, color.y, color.z, 1.0f});
    registry.assign<Transform>(light, glm::scale(glm::translate(pos), lightMeshScalingFactor));

    auto light2 = registry.create();
    pos = Position{-2.0f, 2.0f, 0.0f};
    color = glm::vec3{1.0f, 1.0f, 1.0f};
    registry.assign<Position>(light2, pos);
    registry.assign<PointLightEmitter>(light2, color, 8.0f);
    registry.assign<Geometry>(light2, cubeGeometry);
    registry.assign<Materials::Basic>(light2, glm::vec4{color.x, color.y, color.z, 1.0f});
    registry.assign<Transform>(light2, glm::scale(glm::translate(pos), lightMeshScalingFactor));
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
}  // namespace bae
