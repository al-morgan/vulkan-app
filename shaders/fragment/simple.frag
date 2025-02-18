#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) readonly buffer NormalBuffer
{
    vec4 normalBuffer[];
};

void main()
{
    vec4 n = normalBuffer[gl_PrimitiveID / 2];
    vec3 light = vec3(.707f, 0.0, .707f);

    float intensity = dot(vec3(n.x, n.y, n.z), light);
    intensity = pow(intensity, 2.2);
    outColor = vec4(intensity, intensity, intensity, 1.0f);
}
