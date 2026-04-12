#version 450

layout (set = 0, binding = 0) uniform UniformBuffer {
    mat4 transform;
} ubo;

layout(push_constant) uniform Push {
    vec4 color;
    uint nodeIndex;
    float transparency;
} pc;



layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec2 uvCoord;
layout(location = 3) in vec4 vertexColor;
layout(location = 4) in uint textureIndex;

layout(location = 5) in mat4 modelTransform;
//layout(location = 7) in uint textureIndex;

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
    gl_Position = ubo.transform * modelTransform * vertexPosition;
    //gl_Position = vec4(vertexPosition, 1.0);
    //gl_Position = vec4(positions[gl_VertexIndex], 1.0);
    //normal = 0.5 * (normalize(vertexNormal) + vec3(1.0, 1.0, 1.0));
    //uvFragCoord = vec3(uvCoord, 0);
    fragUV = uvCoord.xy;
    color = vertexColor;
    intensity = min(max(dot(normalize(vertexNormal.xyz), sunVektor), 0.3) + 0.2, 1.0);
    //uvFragCoord = vec2(0.5, 0.5);
    texIndex = textureIndex;
}