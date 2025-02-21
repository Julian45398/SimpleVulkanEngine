#version 450


//layout (set = 1, binding = 0) uniform sampler2D texSampler;
//layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(set = 0, binding = 1) uniform sampler texSampler;
layout(set = 0, binding = 2) uniform texture2D textures[128];

layout(push_constant) uniform PER_OBJECT
{
	int imgIdx;
}pc;


layout(location = 0) in vec2 fragUV;
layout(location = 1) in float color;
layout(location = 2) flat in uint texIndex;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(sampler2D(textures[texIndex + pc.imgIdx], texSampler), fragUV) * color;
    //outColor = texture(texSampler, uvFragCoord) * color;
    outColor[3] = 1.0;
    //outColor = vec4(color, 1.0);
    //outColor = vec4(0.2, 0.3, 0.1, 1.0);
}