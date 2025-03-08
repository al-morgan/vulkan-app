#version 450

layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_position;


layout(location = 0) out vec4 outColor;


layout(set = 0, binding = 2) readonly buffer NormalBuffer
{
    vec4 normalBuffer[];
};

layout(set = 1, binding = 0) uniform ubo
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

void main()
{
    vec3 light = vec3(.707f, 0.0, .707f);
    vec3 light_position = vec3(500, 500, 500);

    vec3 direction = normalize(-(in_position - light_position));

    float intensity = dot(vec3(in_normal.x, in_normal.y, in_normal.z), direction);
    intensity = max(intensity, 0.0);
    outColor = vec4(intensity, intensity, intensity, 1.0);
}
