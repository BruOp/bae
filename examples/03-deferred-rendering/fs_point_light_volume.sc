$input v_screenPos

#include "../common/common.sh"
#include "../common/pbr_helpers.sh"

#define MAX_LIGHT_COUNT 255

SAMPLER2D(diffuseRT, 0);
SAMPLER2D(normalRT, 1);
SAMPLER2D(depthRT, 2);

uniform vec4 cameraPos;
uniform vec4 lightColorIntensity;
uniform vec4 lightPosRadius;

vec3 specular(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 baseColor, vec3 OccRoughMetal) {
    vec3 h = normalize(lightDir + viewDir);
    float NoV = clamp(dot(normal, viewDir), 1e-5, 1.0);
    float NoL = clampDot(normal, lightDir);
    float NoH = clampDot(normal, h);
    float VoH = clampDot(viewDir, h);

    // Needs to be a uniform

    float metallic = OccRoughMetal.z;
    float roughness = OccRoughMetal.y;
    float D = D_GGX(NoH, roughness);
    vec3  F = F_Schlick(VoH, metallic, baseColor);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);
    return D * V * F;
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
        diffuseColor(diffuse, OccRoughMetal.z) +
        PI * specular(lightDir, viewDir, normal, diffuse, OccRoughMetal)
    ) * light;

    gl_FragColor = vec4(color, 1.0);
}
