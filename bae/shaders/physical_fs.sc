$input v_position, v_normal

#define MAX_LIGHT_COUNT 10

// Scene
uniform vec4 cameraPos;
uniform vec4 pointLight_params;
uniform vec4 pointLight_colorIntensity[MAX_LIGHT_COUNT];
uniform vec4 pointLight_pos[MAX_LIGHT_COUNT];

// Material
uniform vec4 matColor;
uniform vec4 metallicRoughnessReflectance;

#include "./common/common.sh"


float D_GGX(float NoH, float linearRoughness) {
    float a = NoH * linearRoughness;
    float k = linearRoughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

float V_SmithGGXCorrelatedFast(float NoV, float NoL, float linearRoughness) {
    float a = linearRoughness;
    float GGXV = NoL * (NoV * (1.0 - a) + a);
    float GGXL = NoV * (NoL * (1.0 - a) + a);
    return 0.5 / (GGXV + GGXL);
}

vec3 F_Schlick(float VoH, float reflectance, float metallic) {
    vec3 f0 = vec3_splat(0.16 * reflectance * reflectance * (1.0 - metallic)) + matColor.xyz * metallic;
    float f = pow(1.0 - VoH, 5.0);
    return f + f0 * (1.0 - f);
}

vec3 specular(vec3 lightDir, vec3 viewDir, vec3 normal) {
    vec3 h = normalize(lightDir + viewDir);
    float NoV = clamp(dot(normal, viewDir), 1e-5, 1.0);
    float NoL = clampDot(normal, lightDir);
    float NoH = clampDot(normal, h);
    float VoH = clampDot(viewDir, h);

    // Needs to be a uniform
    float metallic = metallicRoughnessReflectance.x;
    float linearRoughness = metallicRoughnessReflectance.y;
    float reflectance = metallicRoughnessReflectance.z;
    float D = D_GGX(NoH, linearRoughness);
    vec3  F = F_Schlick(VoH, reflectance, metallic);
    float V = V_SmithGGXCorrelatedFast(NoV, NoL, linearRoughness);
    return vec3(D * V * F);// * V * F;
}

vec3 diffuseBRDF(vec3 color, float metallic) {
    return color * (1.0 - metallic);
}

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 viewDir = normalize(cameraPos.xyz - v_position);

    vec3 color = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < MAX_LIGHT_COUNT; i++) {
        vec3 lightDir = pointLight_pos[i].xyz - v_position;
        float _distance = length(lightDir);
        lightDir = normalize(lightDir);

        float attenuation = pointLight_colorIntensity[i].w / (_distance * _distance);
        vec3 light = attenuation * pointLight_colorIntensity[i].xyz * clampDot(normal, lightDir);

        color += (diffuseBRDF(matColor.xyz, metallicRoughnessReflectance.x) + PI * specular(lightDir, viewDir, normal)) * light;
    }

    gl_FragColor = vec4(toGamma(color), 1.0);
}
