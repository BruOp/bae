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
#include "Materials.h"
#include "ModelLoader.h"
#include "SceneUniforms.h"
#include "Window.h"
#include "utils/Vertex.h"

#if BX_PLATFORM_WINDOWS
#define SDL_MAIN_HANDLED
#endif // BX_PLATFORM_WINDOWS

namespace bae {
class Renderer {
public:
    Renderer() = default;
    ~Renderer() noexcept;

    void init(Window* pWindow) noexcept;
    void renderFrame(const float dt, const float _time, Camera& camera, entt::DefaultRegistry& registry);

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
    bae::Window* pWindow = nullptr;
    uint64_t state = 0;

    Instance instance = Instance{};

    GeometryRegistry geoRegistry;
    LightUniformSet pointLightUniforms = { "pointLight" };
    SceneUniforms sceneUniforms;
    MaterialTypeManager matTypeManager;

private:
    template <typename Material>
    void renderMaterialCollection(entt::DefaultRegistry& registry, const bgfx::ViewId viewId, const uint64_t state)
    {
        const bgfx::ProgramHandle program = Material::materialType.program;
        registry.view<Position, Geometry, Material>().each(
            [program, viewId, state](const auto&, const Position& pos, const auto& geo, const auto& mat) {
                glm::mat4 mtx = glm::translate(glm::identity<glm::mat4>(), pos);
                bgfx::setTransform(glm::value_ptr(mtx));

                geo.set(viewId);
                mat.setUniforms();
                bgfx::setState(state);
                bgfx::submit(viewId, program);
            });
    }
};
} // namespace bae
