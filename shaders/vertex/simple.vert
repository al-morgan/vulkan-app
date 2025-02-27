#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 normal;

// layout(location = 0) out vec4 fragColor;
layout(location = 0) out float intensity;
layout(location = 2) out vec3 outNormal;

layout(set = 1, binding = 0) uniform uniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    outNormal = normal;

    vec3 n = normal;
    vec3 light = vec3(.707f, 0.0, .707f);
    intensity = dot(vec3(n.x, n.y, n.z), light);
}
