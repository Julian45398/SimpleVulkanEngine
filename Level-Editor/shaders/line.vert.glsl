#version 450


layout (set = 0, binding = 0) uniform UniformBuffer {
    mat4 transform;
} ubo;

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 0) out vec4 color;


//vec3 positions[4] = vec3[](
    //vec3(-0.5f, 0.5f, 0.5f), 
	//vec3(-0.5f, -0.5f, 0.5f),
	//vec3(0.5f, -0.5f, 0.5f),
	//vec3(0.5f, 0.5f, 0.5f)
//);

void main() {
    gl_PointSize = 40.f;
    gl_Position = ubo.transform * vertexPosition;
    color = vertexColor;
}