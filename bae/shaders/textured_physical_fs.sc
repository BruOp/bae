$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord

#define MAX_LIGHT_COUNT 10
#define DIELECTRIC_SPECULAR 0.04
#define BLACK vec3(0.0, 0.0, 0.0)
// Scene
uniform vec4 cameraPos;
uniform vec4 pointLight_params;
uniform vec4 pointLight_colorIntensity[MAX_LIGHT_COUNT];
uniform vec4 pointLight_pos[MAX_LIGHT_COUNT];

#include "./common/common.sh"

// Material
SAMPLER2D(baseColor, 0);
SAMPLER2D(normalMap, 1);
SAMPLER2D(occlusionRoughnessMetalness, 2);


float D_GGX(float NoH, float linearRoughness) {
    float a = NoH * linearRoughness;
    float k = linearRoughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

float V_SmithGGXCorrelatedFast(float NoV, float NoL, float a) {
    // a = linearRoughness;
    float GGXV = NoL * (NoV * (1.0 - a) + a);
    float GGXL = NoV * (NoL * (1.0 - a) + a);
    return 0.5 / (GGXV + GGXL);
}

vec3 F_Schlick(float VoH, float reflectance, float metallic, vec3 baseColor) {
    vec3 f0 = mix(vec3_splat(reflectance), baseColor, metallic);
    // vec3 f0 = vec3_splat(0.16 * reflectance * reflectance * (1.0 - metallic)) + baseColor * metallic;
    float f = pow(1.0 - VoH, 5.0);
    return f + f0 * (1.0 - f);
}

vec3 specular(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 baseColor, vec4 OccRoughMetal) {
    vec3 h = normalize(lightDir + viewDir);
    float NoV = clamp(dot(normal, viewDir), 1e-5, 1.0);
    float NoL = clampDot(normal, lightDir);
    float NoH = clampDot(normal, h);
    float VoH = clampDot(viewDir, h);

    // Needs to be a uniform

    float metallic = OccRoughMetal.z;
    float linearRoughness = OccRoughMetal.y;
    float D = D_GGX(NoH, linearRoughness);
    vec3  F = F_Schlick(VoH, DIELECTRIC_SPECULAR, metallic, baseColor);
    float V = V_SmithGGXCorrelatedFast(NoV, NoL, linearRoughness);
    return vec3(D * V * F);// * V * F;
}

vec3 diffuseBRDF(vec3 color, float metallic) {
    return mix(color * (1.0 - DIELECTRIC_SPECULAR), BLACK, metallic);
}

void main()
{
    mat3 tbn = mat3(v_bitangent, v_tangent, v_normal);
    vec3 normal = normalize(texture2D(normalMap, v_texcoord).xyz * 2.0 - 1.0);
    normal = mul(tbn, normal);
    vec3 viewDir = normalize(cameraPos.xyz - v_position);

    vec3 color = vec3(0.0, 0.0, 0.0);
    vec4 matColor = toLinear(texture2D(baseColor, v_texcoord));
    vec4 OccRoughMetal = toLinearAccurate(texture2D(occlusionRoughnessMetalness, v_texcoord));
    for (int i = 0; i < MAX_LIGHT_COUNT; i++) {
        vec3 lightDir = pointLight_pos[i].xyz - v_position;
        float _distance = length(lightDir);
        lightDir = normalize(lightDir);

        float attenuation = pointLight_colorIntensity[i].w / (_distance * _distance);
        vec3 light = attenuation * pointLight_colorIntensity[i].xyz * clampDot(normal, lightDir);

        color += (
            diffuseBRDF(matColor.xyz, OccRoughMetal.z) +
            PI * specular(lightDir, viewDir, normal, matColor.xyz, OccRoughMetal)
        ) * light;
    }

    gl_FragColor = vec4(toGamma(color), 1.0);
}