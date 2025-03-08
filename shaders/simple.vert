#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 in_normal;
layout(location = 2) out vec3 out_normal;
layout(location = 3) out vec3 out_position;

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
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
    out_normal = in_normal;
    out_position = inPosition;
}
