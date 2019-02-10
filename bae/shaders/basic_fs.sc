$input v_position, v_normal

#define MAX_LIGHT_COUNT 10

// Scene
uniform vec4 cameraPos;
uniform vec4 pointLight_params;
uniform vec4 pointLight_colorIntensity[MAX_LIGHT_COUNT];
uniform vec4 pointLight_pos[MAX_LIGHT_COUNT];

// Material
uniform vec4 matColor;

// #include "./common/common.sh"
#include <bgfx_shader.sh>

#define PI 3.141592653589793
#define INV_PI 0.3183098861837907

vec3 diffuseBRDF() {
    return matColor.xyz * INV_PI;
}

vec3 toGamma(vec3 _rgb)
{
	return pow(abs(_rgb), vec3_splat(1.0/2.2) );
}

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 color = vec3(0.0, 0.0, 0.0);

    // int lightCount = 2;
    // lightCount = min(lightCount, MAX_LIGHT_COUNT);

    for (int i = 0; i < MAX_LIGHT_COUNT; i++) {
        vec3 lightDir = pointLight_pos[i].xyz - v_position;
        float _distance = length(lightDir);
        lightDir = normalize(lightDir);

        float attenuation = pointLight_colorIntensity[i].w / (_distance * _distance);
        vec3 light = attenuation * pointLight_colorIntensity[i].xyz * clamp(dot(normal, lightDir), 0.0, 1.0);

        color += diffuseBRDF() * light;
    }

    gl_FragColor = vec4(toGamma(color), 1.0);
}
