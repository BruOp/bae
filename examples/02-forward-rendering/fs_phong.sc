$input v_position, v_texcoord, v_normal, v_tangent, v_bitangent

#include "../common/common.sh"

#define MAX_LIGHT_COUNT 255

uniform vec4 cameraPos;
uniform vec4 pointLight_params;
uniform vec4 pointLight_colorIntensity[MAX_LIGHT_COUNT];
uniform vec4 pointLight_pos[MAX_LIGHT_COUNT];

SAMPLER2D(diffuseMap, 0);
SAMPLER2D(normalMap, 1);
SAMPLER2D(specularMap, 2);

float clampDot(vec3 v1, vec3 v2) {
    return clamp(dot(v1, v2), 0.0, 1.0);
}

vec3 specularBRDF(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 matColor, float specular) {
    vec3 h = normalize(lightDir + viewDir);
    float NoH = clampDot(normal, h);
    return specular * pow(NoH, 16.0) * matColor;
}

void main()
{
    mat3 tbn = mat3FromCols(
        normalize(v_tangent),
        normalize(v_bitangent),
        normalize(v_normal)
    );
    vec3 normal = normalize(texture2D(normalMap, v_texcoord).xyz * 2.0 - 1.0);
    normal = mul(tbn, normal);

    vec3 viewDir = normalize(cameraPos.xyz - v_position);

    vec4 matColor = texture2D(diffuseMap, v_texcoord);
    float specular = texture2D(specularMap, v_texcoord).r;

    vec3 color = vec3_splat(0.0);

    uint numLights = min(uint(pointLight_params.x), MAX_LIGHT_COUNT);

    for (uint i = 0; i < numLights; i++) {
        vec3 lightPos = pointLight_pos[i].xyz;
        vec4 colorIntensity = pointLight_colorIntensity[i];

        vec3 lightDir = lightPos - v_position;
        float dist = length(lightDir);
        lightDir = lightDir / dist;

        float attenuation = colorIntensity.w / (dist * dist);
        vec3 light = attenuation * colorIntensity.xyz * clampDot(normal, lightDir);

        color += (
            matColor.xyz +
            PI * specularBRDF(lightDir, viewDir, normal, matColor.xyz, specular)
        ) * light;
    }

    color = toGammaAccurate(color * matColor.w);

    gl_FragColor = vec4(color, matColor.w);
}
