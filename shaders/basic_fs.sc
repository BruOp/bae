#include <bgfx_shader.sh>

// Material
uniform vec4 matColor;

void main()
{
    gl_FragColor = matColor;
}
