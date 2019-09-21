$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord

#include "../common/common.sh"
#include "../common/pbr_helpers.sh"

#define MAX_SLOPE_OFFSET 2.0
#define NUM_CASCADES 4

// Scene
uniform vec4 u_shadowMapParams;
#define u_shadowBias u_shadowMapParams.x
#define u_slopeScaleBias u_shadowMapParams.y
#define u_normalOffsetFactor u_shadowMapParams.z
#define u_shadowMapTexelSize u_shadowMapParams.w

uniform vec4 u_cameraPos;
uniform vec4 u_directionalLightParams[2];
#define u_lightColor u_directionalLightParams[0].xyz
#define u_lightIntesity u_directionalLightParams[0].w
#define u_lightDir u_directionalLightParams[1].xyz
uniform vec4 u_samplingDisk[8];
uniform vec4 u_cascadeBounds[NUM_CASCADES];
#define u_diskSize u_cascadeBounds[0].w
uniform mat4 u_lightViewProj[NUM_CASCADES];

SAMPLER2D(s_shadowMap_1, 5);
SAMPLER2D(s_shadowMap_2, 6);
SAMPLER2D(s_shadowMap_3, 7);
SAMPLER2D(s_shadowMap_4, 8);
SAMPLER2D(s_randomTexture, 9);


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


// Taken from http://the-witness.net/news/2013/09/shadow-mapping-summary-part-1/
vec2 get_shadow_offsets(vec3 normal, vec3 lightDir) {
    float NoL = clampDot(normal, lightDir);
    float N_offset_scale = sqrt(1.0 - NoL * NoL); // sin(acos(L·N))
    float L_offset_scale = N_offset_scale / NoL;    // tan(acos(L·N))
    return vec2(N_offset_scale, min(MAX_SLOPE_OFFSET, L_offset_scale));
}


uint getShadowCascadeIdx(float fragDepth) {
    UNROLL
    for (uint i = 0; i < NUM_CASCADES; ++i) {
        if (fragDepth < u_cascadeBounds[i].z) {
            return i;
        }
    }
    return NUM_CASCADES - 1;
}


float sampleLightDepth(
    uint cascadeIdx,
    vec2 texCoords,
    vec2 diskRotation,
    float lightDepth,
    uint i, uint j
) {
    vec2 diskOffset = (
        diskRotation.x * u_samplingDisk[i][j] - diskRotation.y * u_samplingDisk[i][j+1],
        diskRotation.y * u_samplingDisk[i][j] + diskRotation.x * u_samplingDisk[i][j+1]
    );
    // vec2 diskOffset = (u_samplingDisk[i][j], u_samplingDisk[i][j+1]);
    vec2 uv = texCoords + u_diskSize * diskOffset / u_cascadeBounds[cascadeIdx].xy;
    float sampledLightDepth;
    if (cascadeIdx == 0) {
        sampledLightDepth = texture2D(s_shadowMap_1, uv).r;
    } else if (cascadeIdx == 1) {
        sampledLightDepth = texture2D(s_shadowMap_2, uv).r;
    } else if (cascadeIdx == 2) {
        sampledLightDepth = texture2D(s_shadowMap_3, uv).r;
    } else {
        sampledLightDepth = texture2D(s_shadowMap_4, uv).r;
    }
    return step(lightDepth, sampledLightDepth);
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

    vec3 normal = texture2D(s_normal, v_texcoord).xyz * 2.0 - 1.0;
    // From the MikkTSpace docs! (check mikktspace.h)
    normal = normalize(normal.x * v_tangent + normal.y * v_bitangent + normal.z * v_normal);

    float selfOcclusion = clampDot(normal, -u_lightDir);
    float shadowVisibility = 0.0;

    if (selfOcclusion > 0.0) {
        // SHADOWING
        uint cascadeIdx = getShadowCascadeIdx(gl_FragCoord.z);
        vec2 offsets = u_shadowMapParams.zy * get_shadow_offsets(normal, u_lightDir);

        // Move along normal in world space
        vec3 samplePosition = v_position + v_normal * offsets.x;
        // Transform to light space
        vec4 lightClip = mul(u_lightViewProj[cascadeIdx], vec4(samplePosition, 1.0));
        vec3 lightUVDepth = lightClip.xyz / lightClip.w;
    #if BGFX_SHADER_LANGUAGE_GLSL
        lightUVDepth = 0.5 * lightClip + 0.5;
    #else
        lightUVDepth.xy = 0.5 * lightUVDepth.xy + 0.5;
        lightUVDepth.y = 1.0 - lightUVDepth.y;
    #endif

        float lightDepth = lightUVDepth.z - u_shadowBias - offsets.y;
        // Red is cos theta, green is sin theta
        vec2 diskRotation = texture2D(s_randomTexture, 0.5 * gl_FragCoord.xy + 0.5).rg;

        UNROLL
        for (uint i = 0; i < 8; i++) {
            shadowVisibility += sampleLightDepth(cascadeIdx, lightUVDepth.xy, diskRotation, lightDepth, i, 0);
            shadowVisibility += sampleLightDepth(cascadeIdx, lightUVDepth.xy, diskRotation, lightDepth, i, 2);
        }
        shadowVisibility /= 16.0;
    }


    vec3 viewDir = normalize(u_cameraPos.xyz - v_position);

    vec2 roughnessMetal = texture2D(s_metallicRoughness, v_texcoord).yz;
    float roughness = max(roughnessMetal.x * u_roughnessFactor, MIN_ROUGHNESS);
    float metallic = roughnessMetal.y * u_metallicFactor;
    float occlusion = texture2D(s_occlusion, v_texcoord).x;
    vec3 emissive = toLinear(texture2D(s_emissive, v_texcoord)).xyz * u_emissiveFactor;
    vec3 light = shadowVisibility * u_lightIntesity * u_lightColor * selfOcclusion;

    vec3 color = 0.1 * diffuseColor(baseColor.xyz, metallic) + (
        diffuseColor(baseColor.xyz, metallic) +
        PI * specular(-u_lightDir, viewDir, normal, baseColor.xyz, roughness, metallic)
    ) * light;

    gl_FragColor = vec4(color, 1.0);
}
