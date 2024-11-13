#version 450


//layout (set = 0, binding = 0) uniform UniformBuffer {
    //mat4 transform;
//} ubo;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in uint ImageIndex;
layout(location = 2) in vec2 uvCoord;
layout(location = 3) in uint animationIndex;
layout(location = 4) in float intensity;

layout(location = 0) out vec2 uvFragCoord;


//vec3 positions[4] = vec3[](
    //vec3(-0.5f, 0.5f, 0.5f), 
	//vec3(-0.5f, -0.5f, 0.5f),
	//vec3(0.5f, -0.5f, 0.5f),
	//vec3(0.5f, 0.5f, 0.5f)
//);

void main() {
    //gl_Position = ubo.transform * vec4(vertexPosition, 1.0);
    gl_Position = vec4(vertexPosition, 1.0);
    //gl_Position = vec4(positions[gl_VertexIndex], 1.0);
    //normal = 0.5 * (normalize(vertexNormal) + vec3(1.0, 1.0, 1.0));
    uvFragCoord = uvCoord;
    //uvFragCoord = vec2(0.5, 0.5);

}