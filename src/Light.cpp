#include "Light.h"

namespace bae
{
void LightUniformSet::init()
{
    auto uniformName = lightName + "_params";
    params = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4);
    uniformName = lightName + "_pos";
    lightPos = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4, maxLightCount);
    uniformName = lightName + "_colorIntensity";
    lightColorIntensity = bgfx::createUniform(uniformName.c_str(), bgfx::UniformType::Vec4, maxLightCount);

    lightPosData.resize(maxLightCount);
    lightColorIntensityData.resize(maxLightCount);
}

void LightUniformSet::set()
{
    float paramsArr[4]{float(lightCount), 0.0f, 0.0f, 0.0f};
    bgfx::setUniform(params, paramsArr);
    bgfx::setUniform(lightPos, lightPosData.data(), maxLightCount);
    bgfx::setUniform(lightColorIntensity, lightColorIntensityData.data(), maxLightCount);
}
}  // namespace bae
