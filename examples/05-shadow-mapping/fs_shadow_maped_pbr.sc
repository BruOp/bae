$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord, v_lightUVDepth

#include "../common/common.sh"
#include "../common/pbr_helpers.sh"

// Scene
uniform vec4 u_cameraPos;
uniform vec4 u_directionalLightParams[2];
#define u_lightColor u_directionalLightParams[0].xyz
#define u_lightIntesity u_directionalLightParams[0].w
#define u_lightDir u_directionalLightParams[1].xyz

SAMPLER2D(s_shadowMap, 5);


// Material
SAMPLER2D(s_baseColor, 0);
SAMPLER2D(s_normal, 1);
SAMPLER2D(s_metallicRoughness, 2);
SAMPLER2D(s_emissive, 3);
SAMPLER2D(s_occlusion, 4);
uniform vec4 u_factors[3];
#define u_baseColorFactor u_factors[0]
#define u_emissiveFactor u_factors[1]
#define u_alphaCutoff u_factors[2].x
#define u_metallicFactor u_factors[2].y
#define u_roughnessFactor u_factors[2].z


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
    vec4 baseColor = toLinear(texture2D(s_baseColor, v_texcoord)) * u_baseColorFactor;
#ifdef MASKING_ENABLED
    if (baseColor.w < u_alphaCutoff) {
        discard;
        return;
    }
#endif // MASKING_ENABLED

    float depth = v_lightUVDepth.z;
    float shadowMapDepth = texture2D(s_shadowMap, v_lightUVDepth.xy).r;

    vec3 normal = texture2D(s_normal, v_texcoord).xyz * 2.0 - 1.0;
    // From the MikkTSpace docs! (check mikktspace.h)
    normal = normalize(normal.x * v_tangent + normal.y * v_bitangent + normal.z * v_normal);

    vec3 viewDir = normalize(u_cameraPos.xyz - v_position);

    vec2 roughnessMetal = texture2D(s_metallicRoughness, v_texcoord).yz;
    float roughness = max(roughnessMetal.x * u_roughnessFactor, MIN_ROUGHNESS);
    float metallic = roughnessMetal.y * u_metallicFactor;
    float occlusion = texture2D(s_occlusion, v_texcoord).x;
    vec3 emissive = toLinear(texture2D(s_emissive, v_texcoord)).xyz * u_emissiveFactor;

    float visibility = (depth - shadowMapDepth) > 0.001 ? 0.0 : 1.0;
    vec3 light = visibility * u_lightIntesity * u_lightColor * clampDot(normal, -u_lightDir);

    vec3 color = 0.1 * diffuseColor(baseColor.xyz, metallic) + (
        diffuseColor(baseColor.xyz, metallic) +
        PI * specular(-u_lightDir, viewDir, normal, baseColor.xyz, roughness, metallic)
    ) * light;

    gl_FragColor = vec4(color, 1.0);
}
