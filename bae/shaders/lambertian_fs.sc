$input v_position, v_normal

#define MAX_LIGHT_COUNT 10

// Scene
uniform vec4 pointLight_params;
uniform vec4 pointLight_colorIntensity[MAX_LIGHT_COUNT];
uniform vec4 pointLight_pos[MAX_LIGHT_COUNT];

// Material
uniform vec4 matColor;

#include "./common/common.sh"

vec3 diffuseBRDF() {
    return matColor.xyz;
}

void main()
{
    vec3 normal = normalize(v_normal);

    vec3 color = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < MAX_LIGHT_COUNT; i++) {
        vec3 lightDir = pointLight_pos[i].xyz - v_position;
        float _distance = length(lightDir);
        lightDir = normalize(lightDir);

        float attenuation = pointLight_colorIntensity[i].w / (_distance * _distance);
        vec3 light = attenuation * pointLight_colorIntensity[i].xyz * clampDot(normal, lightDir);

        color += diffuseBRDF() * light;
    }

    gl_FragColor = vec4(toGamma(color), 1.0);
}
