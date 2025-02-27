#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 in_normal;

layout(location = 2) out vec3 out_normal;

layout(set = 1, binding = 0) uniform uniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    out_normal = in_normal;
}
