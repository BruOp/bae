$input a_position, a_normal
$output v_color0

#define MAX_LIGHT_COUNT 10

uniform vec4 pointLight_params;
uniform vec4 pointLight_lightColorIntensity[MAX_LIGHT_COUNT];
uniform vec4 pointLight_lightPos[MAX_LIGHT_COUNT];

#include <bgfx_shader.sh>

void main()
{
    vec4 position = mul(u_model[0], vec4(a_position, 1.0));
    gl_Position = mul(u_modelViewProj, position);
    vec3 color = vec3(0.0);

    int lightCount = min(2, MAX_LIGHT_COUNT);
    for (int i = 0; i < lightCount; ++i) {
        float _distance = distance(pointLight_lightPos[i].xyz, position.xyz);
        color += pointLight_lightColorIntensity[i].xyz / _distance;
    }

    v_color0 = vec4(color, 1.0);

}
