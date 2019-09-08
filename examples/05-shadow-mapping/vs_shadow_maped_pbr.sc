$input a_position, a_normal, a_texcoord0, a_tangent
$output v_position, v_texcoord, v_normal, v_tangent, v_bitangent, v_lightUVDepth


#include "../common/common.sh"

uniform mat4 u_normalTransform;
uniform mat4 u_lightViewProj;

void main()
{
    v_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    vec4 lightClip = mul(u_lightViewProj, vec4(v_position, 1.0));
    v_lightUVDepth = lightClip.xyz / lightClip.w;
#if BGFX_SHADER_LANGUAGE_GLSL
    v_lightUVDepth = 0.5 * lightClip + 0.5;
#else
    v_lightUVDepth.xy = 0.5 * v_lightUVDepth.xy + 0.5;
    v_lightUVDepth.y = 1.0 - v_lightUVDepth.y;
#endif


    v_normal    = normalize(mul(u_normalTransform, vec4(a_normal, 0.0)).xyz);
    v_tangent   = normalize(mul(u_model[0], vec4(a_tangent.xyz, 0.0)).xyz);
    v_bitangent = normalize(cross(v_normal, v_tangent)) * a_tangent.w;

    v_texcoord = a_texcoord0;

    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
