$input v_position, v_texcoord, v_normal, v_tangent, v_bitangent

#include "../common/common.sh"

uniform vec4 cameraPos;

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
    // TODO put these in a uniform
    const vec3 lightPos = vec3(0.0, 50.0, 0.0);
    const vec4 colorIntensity = vec4(1.0, 1.0, 1.0, 1000.0);

    mat3 tbn = mat3(
        normalize(v_tangent),
        normalize(v_bitangent),
        normalize(v_normal)
    );
    vec3 normal = normalize(texture2D(normalMap, v_texcoord).xyz * 2.0 - 1.0);
    normal = mul(tbn, normal);
    // vec3 normal = normalize(v_normal);

    vec3 viewDir = normalize(cameraPos.xyz - v_position);

    vec3 lightDir = lightPos - v_position;
    float dist = length(lightDir);
    lightDir = lightDir / dist;


    float attenuation = colorIntensity.w / (dist * dist);
    vec3 light = attenuation * colorIntensity.xyz * clampDot(normal, lightDir);

    vec4 matColor = texture2D(diffuseMap, v_texcoord);
    float specular = texture2D(specularMap, v_texcoord).r;

    vec3 color = (
        matColor.xyz +
        PI * specularBRDF(lightDir, viewDir, normal, matColor.xyz, specular)
    ) * light;

    gl_FragColor = vec4(toGamma(color), 1.0);
}
