$input v_position, v_normal, v_tangent, v_bitangent, v_texcoord

#include "../common/common.sh"
#include "../common/pbr_helpers.sh"

#define MAX_LIGHT_COUNT 255

// Scene
uniform vec4 u_cameraPos;
uniform vec4 pointLight_params;
uniform vec4 pointLight_colorIntensity[MAX_LIGHT_COUNT];
uniform vec4 pointLight_pos[MAX_LIGHT_COUNT];


// Material
SAMPLER2D(s_baseColor, 0);
SAMPLER2D(s_normal, 1);
SAMPLER2D(s_metallicRoughness, 2);


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
    return vec3(D * V * F);
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
    vec3 normal = texture2D(s_normal, v_texcoord).xyz * 2.0 - 1.0;
    normal = normalize(mul(tbn, normal));

    vec3 viewDir = normalize(u_cameraPos.xyz - v_position);

    vec3 color = vec3(0.0, 0.0, 0.0);
    vec4 baseColor = texture2D(s_baseColor, v_texcoord);
    vec3 OccRoughMetal = texture2D(s_metallicRoughness, v_texcoord).xyz;

    int numLights = min(floatBitsToUint(pointLight_params.x), MAX_LIGHT_COUNT);

    for (uint i = 0; i < numLights; i++) {
        vec3 lightPos = pointLight_pos[i].xyz;
        float lightRadius = pointLight_pos[i].w;
        vec4 colorIntensity = pointLight_colorIntensity[i];
        vec3 lightDir = lightPos - v_position;
        float dist = length(lightDir);
        lightDir = lightDir / dist;

        float attenuation = karisFalloff(dist, lightRadius) * colorIntensity.w;
        if (attenuation == 0.0) {
            continue;
        }

        vec3 light = attenuation * colorIntensity.xyz * clampDot(normal, lightDir);

        color += (
            diffuseColor(baseColor.xyz, OccRoughMetal.z) +
            PI * specular(lightDir, viewDir, normal, baseColor.xyz, OccRoughMetal)
        ) * light;
    }
    gl_FragColor = vec4(color, baseColor.w);
}
