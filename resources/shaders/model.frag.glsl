#version 450


//layout (set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 uvFragCoord;
layout(location = 1) in float color;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, uvFragCoord) * color;
    outColor[3] = 1.0;
    //outColor = vec4(color, 1.0);
    //outColor = vec4(0.2, 0.3, 0.1, 1.0);
}