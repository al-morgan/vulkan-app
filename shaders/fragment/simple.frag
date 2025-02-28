#version 450

layout(location = 2) in vec3 in_normal;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) readonly buffer NormalBuffer
{
    vec4 normalBuffer[];
};

void main()
{
    vec3 light = vec3(.707f, 0.0, .707f);
    float intensity = dot(vec3(in_normal.x, in_normal.y, in_normal.z), light);
    intensity = max(intensity, 0.0);
    outColor = vec4(intensity, intensity, intensity, 1.0);
}
