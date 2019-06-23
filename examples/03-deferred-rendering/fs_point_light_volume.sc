$input v_screenPos

#include "../common/common.sh"

#define MAX_LIGHT_COUNT 255
#define DIELECTRIC_SPECULAR 0.04
#define BLACK vec3(0.0, 0.0, 0.0)

SAMPLER2D(diffuseRT, 0);
SAMPLER2D(normalRT, 1);
SAMPLER2D(depthRT, 2);

uniform vec4 cameraPos;
uniform vec4 lightColorIntensity;
uniform vec4 lightPosRadius;


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
    vec2 texcoord = gl_FragCoord.xy / u_viewRect.zw;
    float depth = texture2D(depthRT, texcoord).r;

#if BGFX_SHADER_LANGUAGE_GLSL
    vec4 clip = vec4(texcoord * 2.0 - 1.0, (2.0 * depth - 1.0), 1.0);
#else
    vec4 clip = vec4(vec2(texcoord.x, 1.0 - texcoord.y) * 2.0 - 1.0, depth, 1.0);
#endif

    vec4 view = mul(u_invViewProj, clip);
    view = view / view.w;
    vec3 position = view.xyz;

    vec4 diffuseRead = texture2D(diffuseRT, texcoord);
    vec4 normalRead = texture2D(normalRT, texcoord);
    vec3 diffuse = diffuseRead.rgb;
    vec3 normal = normalRead.rgb;

    vec3 viewDir = normalize(cameraPos.xyz - position);

    vec3 OccRoughMetal = vec3(0.0, diffuseRead.a, normalRead.a);

    vec3 lightPos = lightPosRadius.xyz;
    float lightRadius = lightPosRadius.w;
    vec3 lightDir = lightPos - position.xyz;
    float dist = length(lightDir);
    lightDir = lightDir / dist;

    float attenuation = lightColorIntensity.w * karisFalloff(dist, lightRadius);
    vec3 light = attenuation * lightColorIntensity.xyz * clampDot(normal, lightDir);

    vec3 color = (
        diffuseBRDF(diffuse, OccRoughMetal.z) +
        PI * specular(lightDir, viewDir, normal, diffuse, OccRoughMetal)
    ) * light;

    gl_FragColor = vec4(color, 1.0);
}
