#ifndef __BAESHADERLIB_SH__
#define __BAESHADERLIB_SH__

#define PI 3.141592653589793

// -------------- UTILS -----------------

float clampDot(vec3 v1, vec3 v2) {
    return clamp(dot(v1, v2), 0.0, 1.0);
}

vec3 toGamma(vec3 _rgb)
{
    return pow(abs(_rgb), vec3_splat(1.0/2.2) );
}

vec3 toLinear(vec3 _rgb)
{
    return pow(abs(_rgb), vec3_splat(2.2) );
}

vec4 toLinear(vec4 _rgba)
{
    return vec4(toLinear(_rgba.xyz), _rgba.w);
}

vec3 toLinearAccurate(vec3 _rgb)
{
    vec3 lo = _rgb / 12.92;
    vec3 hi = pow( (_rgb + 0.055) / 1.055, vec3_splat(2.4) );
    vec3 rgb = mix(hi, lo, vec3(lessThanEqual(_rgb, vec3_splat(0.04045) ) ) );
    return rgb;
}

vec4 toLinearAccurate(vec4 _rgba)
{
    return vec4(toLinearAccurate(_rgba.xyz), _rgba.w);
}


vec3 convertRGB2XYZ(vec3 _rgb)
{
    // Reference:
    // RGB/XYZ Matrices
    // http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
    vec3 xyz;
    xyz.x = dot(vec3(0.4124564, 0.3575761, 0.1804375), _rgb);
    xyz.y = dot(vec3(0.2126729, 0.7151522, 0.0721750), _rgb);
    xyz.z = dot(vec3(0.0193339, 0.1191920, 0.9503041), _rgb);
    return xyz;
}

vec3 convertXYZ2RGB(vec3 _xyz)
{
    vec3 rgb;
    rgb.x = dot(vec3( 3.2404542, -1.5371385, -0.4985314), _xyz);
    rgb.y = dot(vec3(-0.9692660,  1.8760108,  0.0415560), _xyz);
    rgb.z = dot(vec3( 0.0556434, -0.2040259,  1.0572252), _xyz);
    return rgb;
}

vec3 convertXYZ2Yxy(vec3 _xyz)
{
    // Reference:
    // http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
    float inv = 1.0/dot(_xyz, vec3(1.0, 1.0, 1.0) );
    return vec3(_xyz.y, _xyz.x*inv, _xyz.y*inv);
}

vec3 convertYxy2XYZ(vec3 _Yxy)
{
    // Reference:
    // http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
    vec3 xyz;
    xyz.x = _Yxy.x*_Yxy.y/_Yxy.z;
    xyz.y = _Yxy.x;
    xyz.z = _Yxy.x*(1.0 - _Yxy.y - _Yxy.z)/_Yxy.z;
    return xyz;
}

vec3 convertRGB2Yxy(vec3 _rgb)
{
    return convertXYZ2Yxy(convertRGB2XYZ(_rgb) );
}

vec3 convertYxy2RGB(vec3 _Yxy)
{
    return convertXYZ2RGB(convertYxy2XYZ(_Yxy) );
}

vec3 convertRGB2Yuv(vec3 _rgb)
{
    vec3 yuv;
    yuv.x = dot(_rgb, vec3(0.299, 0.587, 0.114) );
    yuv.y = (_rgb.x - yuv.x)*0.713 + 0.5;
    yuv.z = (_rgb.z - yuv.x)*0.564 + 0.5;
    return yuv;
}

vec3 convertYuv2RGB(vec3 _yuv)
{
    vec3 rgb;
    rgb.x = _yuv.x + 1.403*(_yuv.y-0.5);
    rgb.y = _yuv.x - 0.344*(_yuv.y-0.5) - 0.714*(_yuv.z-0.5);
    rgb.z = _yuv.x + 1.773*(_yuv.z-0.5);
    return rgb;
}

vec3 convertRGB2YIQ(vec3 _rgb)
{
    vec3 yiq;
    yiq.x = dot(vec3(0.299,     0.587,     0.114   ), _rgb);
    yiq.y = dot(vec3(0.595716, -0.274453, -0.321263), _rgb);
    yiq.z = dot(vec3(0.211456, -0.522591,  0.311135), _rgb);
    return yiq;
}

vec3 convertYIQ2RGB(vec3 _yiq)
{
    vec3 rgb;
    rgb.x = dot(vec3(1.0,  0.9563,  0.6210), _yiq);
    rgb.y = dot(vec3(1.0, -0.2721, -0.6474), _yiq);
    rgb.z = dot(vec3(1.0, -1.1070,  1.7046), _yiq);
    return rgb;
}

// ---------------LIGHTING---------------

vec4 getLightDirDistance(vec3 pos, vec3 lightPos) {
    vec3 pointToLight = lightPos - pos;
    float _distance = length(pointToLight);
    return vec4(pointToLight / _distance, _distance);
}

#endif // __BAESHADERLIB_SH__
