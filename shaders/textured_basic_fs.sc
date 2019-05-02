$input v_texcoord

#include <bgfx_shader.sh>

// Material
SAMPLER2D(baseColor, 0);

void main()
{
    vec3 color = texture2D(baseColor, v_texcoord).xyz;
    gl_FragColor = vec4(color, 1.0);
}
