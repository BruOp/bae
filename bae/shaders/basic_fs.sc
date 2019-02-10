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

float D_GGX(float NoH, float linearRoughness) {
    float a = NoH * linearRoughness;
    float k = linearRoughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

float V_SmithGGXCorrelatedFast(float NoV, float NoL, float linearRoughness) {
    float a = linearRoughness;
    float GGXV = NoL * (NoV * (1.0 - a) + a);
    float GGXL = NoV * (NoL * (1.0 - a) + a);
    return 0.5 / (GGXV + GGXL);
}

vec3 F_Schlick(float VoH, vec3 f0) {
    float f = pow(1.0 - VoH, 5.0);
    return f + f0 * (1.0 - f);
}

float clampDot(vec3 v1, vec3 v2) {
    return clamp(dot(v1, v2), 0.0, 1.0);
}

vec3 specular(vec3 lightDir, vec3 viewDir, vec3 normal) {
    vec3 h = normalize(lightDir + viewDir);
    float NoV = abs(dot(normal, viewDir)) + 1e-5;
    float NoL = clampDot(normal, lightDir);
    float NoH = clampDot(normal, h);
    float LoH = clampDot(lightDir, h);

    // Needs to be a uniform
    float linearRoughness = 0.01;
    vec3 f0 = vec3(0.97, 0.96, 0.91);
    float D = D_GGX(NoH, linearRoughness);
    vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelatedFast(NoV, NoL, linearRoughness);
    return D * V * F;
}

vec3 diffuseBRDF() {
    return matColor.xyz;
}

vec3 toGamma(vec3 _rgb)
{
	return pow(abs(_rgb), vec3_splat(1.0/2.2) );
}

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 viewDir = normalize(cameraPos.xyz - v_position);

    // int lightCount = 2;
    // lightCount = min(lightCount, MAX_LIGHT_COUNT);

    vec3 color = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < MAX_LIGHT_COUNT; i++) {
        vec3 lightDir = pointLight_pos[i].xyz - v_position;
        float _distance = length(lightDir);
        lightDir = normalize(lightDir);



        float attenuation = pointLight_colorIntensity[i].w / (_distance * _distance);
        vec3 light = attenuation * pointLight_colorIntensity[i].xyz * clamp(dot(normal, lightDir), 0.0, 1.0);

        color += (diffuseBRDF() + PI * specular(lightDir, viewDir, normal)) * light;
    }

    gl_FragColor = vec4(toGamma(color), 1.0);
}
