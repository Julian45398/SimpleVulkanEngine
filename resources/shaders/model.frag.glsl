#version 450


layout (set = 0, binding = 1) uniform sampler2DArray texSampler;

layout(location = 0) in vec3 uvFragCoord;
layout(location = 1) in vec3 color;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, uvFragCoord);
    //outColor = vec4(color, 1.0);
    //outColor = vec4(0.2, 0.3, 0.1, 1.0);
}