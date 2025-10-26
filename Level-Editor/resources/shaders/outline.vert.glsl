#version 450


layout (set = 0, binding = 0) uniform UniformBuffer {
    mat4 transform;
} ubo;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec2 uvCoord;
layout(location = 3) in vec4 vertexColor;
layout(location = 4) in uint textureIndex;

layout(location = 5) in mat4 modelTransform;
//layout(location = 7) in uint textureIndex;

void main() {
    // Transform to world space
    vec4 worldPos = modelTransform * vec4(vertexPosition, 1.0);
    vec3 worldNormal = normalize(mat3(modelTransform) * vertexNormal.xyz);

    // Get eye-space position
    vec4 viewPos = ubo.transform * worldPos;
    float depth = abs(viewPos.z);

    // Adjust thickness inversely proportional to distance
    float distanceScale = clamp(1.0 / depth, 0.0, 1.0);
    float thickness = distanceScale * 0.5; // tune multiplier

    // Expand vertex
    //vec3 expandedPos = worldPos.xyz + worldNormal * 0.2;
    vec3 expandedPos = vertexPosition + vertexNormal.xyz * 0.1;
    gl_Position = ubo.transform * modelTransform * vec4(expandedPos, 1.0);
}