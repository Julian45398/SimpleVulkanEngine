#version 450


//layout (set = 1, binding = 0) uniform sampler2D texSampler;
//layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec4 color;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = color;
}