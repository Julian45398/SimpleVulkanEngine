#version 450

layout(binding = 0) uniform CameraData {
    mat4 viewProj;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() {
    gl_Position = camera.viewProj * vec4(inPosition, 1.0);
    outColor = inColor;
}