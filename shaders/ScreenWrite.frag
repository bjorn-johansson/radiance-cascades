#version 430 core
in vec2 texCoords;
out vec4 fragColor;

layout(binding = 3) uniform sampler2D _Texture;

void main() {
    fragColor = texture(_Texture, texCoords);
}