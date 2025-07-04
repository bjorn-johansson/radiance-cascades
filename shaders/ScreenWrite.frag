#version 430 core

in vec2 texCoords;
out vec4 fragColor;

layout(binding = 2) uniform sampler2DArray _Texture;

const int PROBE_TEXTURE_SIDE = 1024;
const int PROBE_BLOCK_SIDES[7] = { 2, 4, 8, 16, 32, 64, 128};

uniform int _Layer;

uniform int _Interpolate;
uniform int _ProbeUV;

//the 4 possible weights when doing grid bilinear merging, results in a slightly square look so can be altered to smooth it out
const vec4 weights[2][2] = vec4[2][2](
vec4[2](vec4(0.5625, 0.1875, 0.1875, 0.0625), vec4(0.1875, 0.5625, 0.0625, 0.1875)),
vec4[2](vec4(0.1875, 0.0625, 0.5625, 0.1875), vec4(0.0625, 0.1875, 0.1875, 0.5625))
);

//used to sample probes of higher cascades, or if we increase ray count in cascade 0 (VERY SLOW AT HIGHER CASCADES)
vec3 SampleProbe(ivec2 probe) {
    ivec2 baseProbeTexel = probe * PROBE_BLOCK_SIDES[_Layer];
    
    vec3 light = vec3(0f);
    for(int x = 0; x < PROBE_BLOCK_SIDES[_Layer]; x++)
        for(int y = 0; y < PROBE_BLOCK_SIDES[_Layer]; y++)
            light += texelFetch(_Texture, ivec3(baseProbeTexel + ivec2(x, y), _Layer), 0).rgb;

    float avg = 1f / (PROBE_BLOCK_SIDES[_Layer] * PROBE_BLOCK_SIDES[_Layer]);
    return avg * light;
}


void main() {
    
    vec2 sourceProbeCoord = (texCoords * PROBE_TEXTURE_SIDE) / PROBE_BLOCK_SIDES[_Layer];     //which source probe the target probe belongs to
    ivec2 base = ivec2(floor(sourceProbeCoord));
    
    //shift the source probe coords to be the 4 source probes which centers are the closest to the center of the target probe.
    //imagine a single source probe that contains a 2x2 block of target probes. we select our 4 source probes in the direction of the target probe from the sources center.
    ivec2 offset = ivec2(1,1) - ivec2(floor((sourceProbeCoord - base) * 2)); //TL:1,1   BR:0,0
    base.x -= offset.x;
    base.y -= offset.y;
    
    vec4 weight = weights[offset.y][offset.x];
    
    vec3 P00 = SampleProbe(base);
    vec3 P10 = SampleProbe(base + ivec2(1, 0));
    vec3 P01 = SampleProbe(base + ivec2(0, 1));
    vec3 P11 = SampleProbe(base + ivec2(1, 1));
    
    vec3 lighting = 
    P00 * weight.x +
    P10 * weight.y +
    P01 * weight.z +
    P11 * weight.w;
    
    lighting = lighting / (lighting + 1.0); //Reinhard tonemapping (bad)
    
    lighting.r = pow(lighting.r, 1/2.2f);   //gamma correction
    lighting.g = pow(lighting.g, 1/2.2f);
    lighting.b = pow(lighting.b, 1/2.2f);
    
    if(_Interpolate == 0){
        lighting = texture(_Texture, vec3(texCoords, _Layer)).rgb * 10;
    }
    
    if(_ProbeUV == 1){
        //vec2 uv = weight.xy;
        vec2 uv = vec2(ivec2(texCoords * 1024) % PROBE_BLOCK_SIDES[_Layer] / float(PROBE_BLOCK_SIDES[_Layer]));
        uv *= 0.5f;
        lighting += vec3(uv, 0);
    }
    
    fragColor = vec4(lighting, 1.0);
}