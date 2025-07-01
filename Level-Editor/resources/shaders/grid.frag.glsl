#version 450

layout(location = 0) in float near; //0.01
layout(location = 1) in float far; //100
layout(location = 2) in vec3 nearPoint;
layout(location = 3) in vec3 farPoint;
layout(location = 4) in mat4 viewProj;
//layout(location = 8) in mat4 fragProj;
layout(location = 0) out vec4 outColor;


vec4 grid(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative * 0.5;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    float alpha = 1 - min(line, 1.0);
    vec4 color = vec4(0.2, 0.2, 0.2, alpha);
    // z axis
    if(fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.z = 1.0;
    // x axis
    if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.x = 1.0;
    return color;
}
float computeDepth(vec3 pos) {
    vec4 clip_space_pos = viewProj * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}
float computeLinearDepth(float depth) {
    float clip_space_depth = depth * 2.0 - 1.0; // put back between -1 and 1
    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near)); // get linear value between 0.01 and 100
    return linearDepth / far; // normalize
}
void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);


    float depth = computeDepth(fragPos3D);
    float linearDepth = computeLinearDepth(depth);
    float fading = max(0, (0.5 - 5 * linearDepth));

    outColor = (grid(fragPos3D, 10) + grid(fragPos3D, 1)) * float(t > 0); // adding multiple resolution for the grid

    gl_FragDepth = depth;
    outColor.a *= fading;
}