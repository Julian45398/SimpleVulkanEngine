#version 450


//layout (set = 1, binding = 0) uniform sampler2D texSampler;
//layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(set = 1, binding = 0) uniform sampler texSampler;
layout(set = 1, binding = 1) uniform texture2D textures[128];

layout(push_constant) uniform Push {
    mat4 model;
    vec4 color;
} pc;

//layout(location = 0) in vec2 fragUV;
//layout(location = 1) in float color;
//layout(location = 2) flat in uint texIndex;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 color;
layout(location = 2) flat in uint texIndex;
layout(location = 3) in float intensity;



layout(location = 0) out vec4 outColor;

void main() {
    //outColor = texture(sampler2D(textures[texIndex], texSampler), fragUV) * color * pc.colorModifier;
    //color.r = 1.0;
    //color.g = 1.0;
    //color.b = 1.0;
    //color.a = 1.0;
    //vec4 color2 = vec4(1.0, 1.0, 1.0, 1.0);
    outColor = mix(texture(sampler2D(textures[texIndex], texSampler), fragUV) * color * intensity, pc.color, pc.color.a);
    //outColor = vec4(0.6, 0.6, 0.6, 1.0);
    //outColor = texture(texSampler, uvFragCoord) * color;
    //outColor = texture(sampler2D(textures[texIndex], texSampler), fragUV) * color * intensity;
    outColor[3] = 1.0;
    //outColor = vec4(color, 1.0);
    //outColor = vec4(0.2, 0.3, 0.1, 1.0);
}