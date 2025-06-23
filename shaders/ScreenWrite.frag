#version 430 core

in vec2 texCoords;
out vec4 fragColor;

layout(binding = 2) uniform sampler2DArray _Texture;

const int PROBE_TEXTURE_SIDE = 512;
const int PROBE_BLOCK_SIDE = 2;
const int PROBE_GRID_SIDE = PROBE_TEXTURE_SIDE / PROBE_BLOCK_SIDE;

const int LAYER = 0;

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
    ivec2 baseProbeTexel = probe * PROBE_BLOCK_SIDE;
    vec3 L0 = texelFetch(_Texture, ivec3(baseProbeTexel + ivec2(0, 0), LAYER), 0).rgb;
    vec3 L1 = texelFetch(_Texture, ivec3(baseProbeTexel + ivec2(1, 0), LAYER), 0).rgb;
    vec3 L2 = texelFetch(_Texture, ivec3(baseProbeTexel + ivec2(0, 1), LAYER), 0).rgb;
    vec3 L3 = texelFetch(_Texture, ivec3(baseProbeTexel + ivec2(1, 1), LAYER), 0).rgb;

    return (L0 + L1 + L2 + L3) * 0.25;
}

void main() {
    vec2 zoomedCoord = texCoords * 0.5f + 0.25f;
    
    vec2 textureCoord = texCoords * float(PROBE_GRID_SIDE);
    ivec2 probe = ivec2(floor(textureCoord));
    probe = clamp(probe, ivec2(0), ivec2(PROBE_GRID_SIDE - 2));

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

    //lighting =  texture(_Texture, vec3(texCoords, LAYER)).rgb * 5;
    
    fragColor = vec4(lighting, 1.0);
}