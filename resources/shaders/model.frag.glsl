#version 450


layout(location = 0) in vec2 uvFragCoord;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(uvFragCoord, 0.1, 1.0);
    //outColor = vec4(0.2, 0.3, 0.1, 1.0);
}