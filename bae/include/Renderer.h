#pragma once
#include <memory>
#include <stdexcept>

#include "SDL.h"
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/timer.h>
#include <entt/entt.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Camera.h"
#include "EventHandlers.h"
#include "EventQueue.h"
#include "FPSControls.h"
#include "Geometry.h"
#include "Instance.h"
#include "Light.h"
#include "MaterialType.h"
#include "ModelLoader.h"
#include "Window.h"
#include "materials/Basic.h"
#include "utils/Vertex.h"

#if BX_PLATFORM_WINDOWS
#define SDL_MAIN_HANDLED
#endif // BX_PLATFORM_WINDOWS

namespace bae {
class Renderer {
public:
    Renderer() = default;
    ~Renderer() noexcept;

    void init(uint32_t width, uint32_t height);
    bool update(const float dt);
    void renderFrame(const float dt, entt::DefaultRegistry& registry);

    template <typename Light>
    void setupLighting(entt::DefaultRegistry& registry, LightUniformSet& uniformSet)
    {
        auto view = registry.view<Position, Light>();
        uniformSet.lightCount = view.size();

        size_t counter = 0;
        view.each(
            [&uniformSet, &counter](const auto&, const Position& pos, const Light& light) {
                uniformSet.lightPosData[counter] = {
                    pos.x, pos.y, pos.z, 0.0f
                };
                uniformSet.lightColorIntensityData[counter] = {
                    light.color.x,
                    light.color.y,
                    light.color.z,
                    light.intensity
                };
                ++counter;
            });
    };

    uint32_t width = 0;
    uint32_t height = 0;
    std::unique_ptr<bae::Window> pWindow = nullptr;
    uint64_t state = 0;

    Instance instance = Instance{};

    EventQueue eventQueue;
    WindowInputHandler windowInputHandler;

    GeometryRegistry geoRegistry;
    LightUniformSet pointLightUniforms = { "pointLight" };
    MaterialTypeManager matTypeManager;

    Camera camera;
    FPSControls cameraControls;
};
} // namespace bae
