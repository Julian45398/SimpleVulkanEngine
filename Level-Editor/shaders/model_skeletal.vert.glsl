#version 450

layout (set = 0, binding = 0) uniform UniformBuffer {
    mat4 transform;
} ubo;

const int MAX_BONE_COUNT = 1024;
layout (set = 2, binding = 0) uniform BoneTransforms {
    mat4 transform[MAX_BONE_COUNT];
} boneTransforms;

layout(push_constant) uniform Push {
    vec4 color;
    uint nodeIndex;
    float transparency;
} pc;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 uvCoord;
layout(location = 3) in vec4 vertexColor;
layout(location = 4) in uint textureIndex;

layout(location = 5) in mat4 modelTransform;
layout(location = 9) in uvec4 boneIndices;
layout(location = 10) in vec4 boneWeights;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 color;
layout(location = 2) out uint texIndex;
layout(location = 3) out float intensity;

//vec3 positions[4] = vec3[](
    //vec3(-0.5f, 0.5f, 0.5f), 
	//vec3(-0.5f, -0.5f, 0.5f),
	//vec3(0.5f, -0.5f, 0.5f),
	//vec3(0.5f, 0.5f, 0.5f)
//);

const vec3 sunVektor = normalize(vec3(0.5, 0.5, 0.5));

void main() {
    mat4 skinMatrix =
        boneWeights.x * boneTransforms.transform[boneIndices.x] +
        boneWeights.y * boneTransforms.transform[boneIndices.y] +
        boneWeights.z * boneTransforms.transform[boneIndices.z] +
        boneWeights.w * boneTransforms.transform[boneIndices.w];

    vec4 skinnedPosition = skinMatrix * vec4(vertexPosition, 1.0);

    vec3 skinnedNormal = normalize(mat3(skinMatrix) * vertexNormal.xyz);

    gl_Position = ubo.transform * modelTransform * skinnedPosition;
    fragUV = uvCoord.xy;
    color = vertexColor;
    texIndex = textureIndex;

    intensity = min(max(dot(skinnedNormal, sunVektor), 0.3) + 0.2, 1.0);
}