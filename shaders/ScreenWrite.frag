#version 430 core

in vec2 texCoords;
out vec4 fragColor;

layout(binding = 2) uniform sampler2DArray _Texture;

const int PROBE_TEXTURE_SIDE = 512;
const int PROBE_BLOCK_SIDE = 2;
const int PROBE_COUNT = PROBE_TEXTURE_SIDE / PROBE_BLOCK_SIDE;
const float TEXEL_SIZE = 1.0 / float(PROBE_TEXTURE_SIDE);
const int LAYER = 0;

vec4 BilinearWeights(vec2 ratio) {
    return vec4(
    (1.0 - ratio.x) * (1.0 - ratio.y),
    ratio.x * (1.0 - ratio.y),
    (1.0 - ratio.x) * ratio.y,
    ratio.x * ratio.y
    );
}

vec3 SampleProbe(vec2 probeUV) {
    vec3 L0 = texture(_Texture, vec3(probeUV + TEXEL_SIZE * vec2(0, 0), LAYER)).rgb;
    vec3 L1 = texture(_Texture, vec3(probeUV + TEXEL_SIZE * vec2(1, 0), LAYER)).rgb;
    vec3 L2 = texture(_Texture, vec3(probeUV + TEXEL_SIZE * vec2(0, 1), LAYER)).rgb;
    vec3 L3 = texture(_Texture, vec3(probeUV + TEXEL_SIZE * vec2(1, 1), LAYER)).rgb;

    return (L0 + L1 + L2 + L3) * 0.25;
}

void main() {
    vec2 probeCoord = texCoords * float(PROBE_COUNT);
    ivec2 base = ivec2(floor(probeCoord));
    base = clamp(base, ivec2(0), ivec2(PROBE_COUNT - 2));

    vec2 frac = fract(probeCoord);
    vec4 weight = BilinearWeights(frac);

    vec2 baseUV = vec2(base) * float(PROBE_BLOCK_SIDE) * TEXEL_SIZE;
    float probeStep = float(PROBE_BLOCK_SIDE) * TEXEL_SIZE;

    vec3 p00 = SampleProbe(baseUV);
    vec3 p10 = SampleProbe(baseUV + vec2(probeStep, 0));
    vec3 p01 = SampleProbe(baseUV + vec2(0, probeStep));
    vec3 p11 = SampleProbe(baseUV + probeStep);

    vec3 lighting =
    p00 * weight.x +
    p10 * weight.y +
    p01 * weight.z +
    p11 * weight.w;
    lighting.r = pow(lighting.r, 1/2.2f);
    lighting.g = pow(lighting.g, 1/2.2f);
    lighting.b = pow(lighting.b, 1/2.2f);
    
    lighting = lighting / (lighting + 1.0); //Reinhard tonemapping (bad)
    
    //gamma correction
    if(lighting.g > 1.f){
        lighting *= 0.f;
        lighting.r = 1.f;
    }
    fragColor = vec4(lighting, 1.0);


    //fragColor = texture(_Texture, vec3(texCoords, LAYER));
}