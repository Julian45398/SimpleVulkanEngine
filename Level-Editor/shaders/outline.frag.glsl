#version 450


//layout (set = 1, binding = 0) uniform sampler2D texSampler;
//layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(set = 1, binding = 0) uniform sampler texSampler;
layout(set = 1, binding = 1) uniform texture2D textures[128];

//layout(push_constant) uniform Push {
    //vec4 color;
    //uint nodeIndex;
    //float transparency;
//} pc;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0.9, 0.5, 0.3, 1.0);
}