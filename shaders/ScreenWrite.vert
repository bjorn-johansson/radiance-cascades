#version 430 core
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTexCoords;

out vec2 texCoords;

void main() {
    texCoords = inTexCoords;
    gl_Position = vec4(inPos.xy, 0.0, 1.0);
}