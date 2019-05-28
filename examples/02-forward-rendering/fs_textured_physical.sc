$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord

#include "../common/common.sh"

#define MAX_LIGHT_COUNT 255
#define DIELECTRIC_SPECULAR 0.04
#define BLACK vec3(0.0, 0.0, 0.0)
// Scene
uniform vec4 cameraPos;
uniform vec4 pointLight_params;
uniform vec4 pointLight_colorIntensity[MAX_LIGHT_COUNT];
uniform vec4 pointLight_pos[MAX_LIGHT_COUNT];


// Material
SAMPLER2D(diffuseMap, 0);
SAMPLER2D(normalMap, 1);
SAMPLER2D(metallicRoughnessMap, 2);


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
    float f = pow(1.0 - VoH, 5.0);
    return f + f0 * (1.0 - f);
}

vec3 specular(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 baseColor, vec3 OccRoughMetal) {
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

float karisFalloff(float dist, float lightRadius) {
    return pow(clamp(1.0 - pow(dist/lightRadius, 4), 0.0, 1.0), 2) / (dist * dist + 1);
}

void main()
{
    mat3 tbn = mat3FromCols(
        normalize(v_tangent),
        normalize(v_bitangent),
        normalize(v_normal)
    );
    vec3 normal = texture2D(normalMap, v_texcoord).xyz * 2.0 - 1.0;
    normal = normalize(mul(tbn, normal));

    vec3 viewDir = normalize(cameraPos.xyz - v_position);

    vec3 color = vec3(0.0, 0.0, 0.0);
    vec4 matColor = texture2D(diffuseMap, v_texcoord);
    vec3 OccRoughMetal = texture2D(metallicRoughnessMap, v_texcoord).xyz;

    int numLights = min(int(pointLight_params.x), MAX_LIGHT_COUNT);

    for (int i = 0; i < numLights; i++) {
        vec3 lightPos = pointLight_pos[i].xyz;
        float lightRadius = pointLight_pos[i].w;
        vec4 colorIntensity = pointLight_colorIntensity[i];
        vec3 lightDir = lightPos - v_position;
        float dist = length(lightDir);
        lightDir = lightDir / dist;

        float attenuation = colorIntensity.w * karisFalloff(dist, lightRadius);
        vec3 light = attenuation * colorIntensity.xyz * clampDot(normal, lightDir);

        color += (
            diffuseBRDF(matColor.xyz, OccRoughMetal.z) +
            PI * specular(lightDir, viewDir, normal, matColor.xyz, OccRoughMetal)
        ) * light;
    }
    color = color * matColor.w;
    gl_FragColor = vec4(color, matColor.w);
}
