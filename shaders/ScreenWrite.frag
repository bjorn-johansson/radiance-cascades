#version 430 core

in vec2 texCoords;
out vec4 fragColor;

layout(binding = 2) uniform sampler2DArray _Texture;

const int PROBE_TEXTURE_SIDE = 512;
const int PROBE_BLOCK_SIDES[7] = { 2, 4, 8, 16, 32, 64, 128};

uniform int _Layer;

uniform int _Interpolate;
uniform int _ProbeUV;


vec4 BilinearWeights(vec2 ratio) {
    return vec4(
    (1.0 - ratio.x) * (1.0 - ratio.y),
    ratio.x * (1.0 - ratio.y),
    (1.0 - ratio.x) * ratio.y,
    ratio.x * ratio.y
    );
}
//texcoords are uv coordinates so using texture() kinda makes sense here? But for now i don't want any kind of filtering applied.
vec3 SampleProbe(ivec2 probe) {
    ivec2 baseProbeTexel = probe * PROBE_BLOCK_SIDES[_Layer];
    vec3 L0 = texelFetch(_Texture, ivec3(baseProbeTexel + ivec2(0, 0), _Layer), 0).rgb;
    vec3 L1 = texelFetch(_Texture, ivec3(baseProbeTexel + ivec2(1, 0), _Layer), 0).rgb;
    vec3 L2 = texelFetch(_Texture, ivec3(baseProbeTexel + ivec2(0, 1), _Layer), 0).rgb;
    vec3 L3 = texelFetch(_Texture, ivec3(baseProbeTexel + ivec2(1, 1), _Layer), 0).rgb;

    return (L0 + L1 + L2 + L3) * 0.25;
}
//vec3 SampleProbe(ivec2 probe) {
//    ivec2 baseProbeTexel = probe * PROBE_BLOCK_SIDES[_Layer];
//    
//    vec3 light = vec3(0f);
//    for(int x = 0; x < PROBE_BLOCK_SIDES[_Layer]; x++)
//        for(int y = 0; y < PROBE_BLOCK_SIDES[_Layer]; y++)
//            light += texelFetch(_Texture, ivec3(baseProbeTexel + ivec2(x, y), _Layer), 0).rgb;
//
//    float avg = 1f / (PROBE_BLOCK_SIDES[_Layer] * PROBE_BLOCK_SIDES[_Layer]);
//    return avg * light;
//}


void main() {
    vec2 zoomedCoord = texCoords * 0.5f + 0.25f;
    
    int probeGridSide = PROBE_TEXTURE_SIDE / PROBE_BLOCK_SIDES[_Layer];
    
    vec2 textureCoord = texCoords * float(probeGridSide - 1);
    ivec2 probe = ivec2(floor(textureCoord));
    probe = clamp(probe, ivec2(0), ivec2(probeGridSide - 2));

    vec2 frac = fract(textureCoord);
    vec4 weight = BilinearWeights(frac);

    vec3 P00 = SampleProbe(probe);
    vec3 P10 = SampleProbe(probe + ivec2(1, 0));
    vec3 P01 = SampleProbe(probe + ivec2(0, 1));
    vec3 P11 = SampleProbe(probe + ivec2(1, 1));

    vec3 lighting = 
    P00 * weight.x +
    P10 * weight.y +
    P01 * weight.z +
    P11 * weight.w;


    
    lighting = lighting / (lighting + 1.0); //Reinhard tonemapping (bad)

    lighting.r = pow(lighting.r, 1/2.2f);   //gamma correction
    lighting.g = pow(lighting.g, 1/2.2f);
    lighting.b = pow(lighting.b, 1/2.2f);

    if(_Interpolate == 0)
        lighting =  texture(_Texture, vec3(texCoords, _Layer)).rgb * 5;
    if(_ProbeUV == 1){
        vec2 uv = frac;
        uv *= 0.5f;
        //lighting *= 0.5f;
        lighting += vec3(uv, 0);
    }
    
    fragColor = vec4(lighting, 1.0);
}