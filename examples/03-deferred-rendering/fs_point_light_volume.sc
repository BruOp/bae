$input v_screenPos

#include "../common/common.sh"
#include "../common/pbr_helpers.sh"

SAMPLER2D(s_baseColorRoughness, 0);
SAMPLER2D(s_normalMetallic, 1);
SAMPLER2D(s_emissiveOcclusion, 2);
SAMPLER2D(s_depth, 3);

uniform vec4 u_cameraPos;
uniform vec4 u_lightColorIntensity;
uniform vec4 u_lightPosRadius;

#define u_lightPos u_lightPosRadius.xyz
#define u_lightRadius u_lightPosRadius.w
#define u_lightColor u_lightColorIntensity.xyz
#define u_lightIntensity u_lightColorIntensity.w

vec3 specular(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 baseColor, float roughness, float metallic) {
    vec3 h = normalize(lightDir + viewDir);
    float NoV = clamp(dot(normal, viewDir), 1e-5, 1.0);
    float NoL = clampDot(normal, lightDir);
    float NoH = clampDot(normal, h);
    float VoH = clampDot(viewDir, h);

    // Needs to be a uniform

    float D = D_GGX(NoH, roughness);
    vec3  F = F_Schlick(VoH, metallic, baseColor);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);
    return vec3(D * V * F);
}

float karisFalloff(float dist, float lightRadius) {
    return pow(clamp(1.0 - pow(dist/lightRadius, 4), 0.0, 1.0), 2) / (dist * dist + 1);
}

void main()
{
    vec2 texcoord = gl_FragCoord.xy / u_viewRect.zw;
    float depth = texture2D(s_depth, texcoord).r;

#if BGFX_SHADER_LANGUAGE_GLSL
    vec4 clip = vec4(texcoord * 2.0 - 1.0, (2.0 * depth - 1.0), 1.0);
#else
    vec4 clip = vec4(vec2(texcoord.x, 1.0 - texcoord.y) * 2.0 - 1.0, depth, 1.0);
#endif

    vec4 view = mul(u_invViewProj, clip);
    view = view / view.w;
    vec3 position = view.xyz;

    vec4 baseColorRoughness = texture2D(s_baseColorRoughness, texcoord);
    vec4 normalMetallic = texture2D(s_normalMetallic, texcoord);

    vec3 baseColor = baseColorRoughness.rgb;
    vec3 normal = normalMetallic.rgb;
    float roughness = max(baseColorRoughness.a, MIN_ROUGHNESS);
    float metallic = normalMetallic.a;
    float occlusion = texture2D(s_emissiveOcclusion, texcoord).a;

    vec3 viewDir = normalize(u_cameraPos.xyz - position);

    vec3 lightDir = u_lightPos - position.xyz;
    float dist = length(lightDir);
    lightDir = lightDir / dist;

    float attenuation = u_lightIntensity * karisFalloff(dist, u_lightRadius);
    vec3 light = attenuation * u_lightColor * clampDot(normal, lightDir);

    vec3 color = (
        diffuseColor(baseColor, metallic) +
        PI * specular(lightDir, viewDir, normal, baseColor, roughness, metallic)
    ) * light;

    gl_FragColor = vec4(color * occlusion, 1.0);
}
