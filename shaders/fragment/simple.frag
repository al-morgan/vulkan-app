#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in float intensity;
layout(location = 2) in vec3 normal;

layout(set = 0, binding = 2) readonly buffer NormalBuffer
{
    vec4 normalBuffer[];
};

void main()
{
    //vec4 n = normalBuffer[gl_PrimitiveID / 2];

    vec3 n = normal;

    vec3 light = vec3(.707f, 0.0, .707f);

//    float intensity = dot(vec3(n.x, n.y, n.z), light);
//    intensity = max(intensity, 0.0);
//    intensity += .02;
//    //intensity = pow(intensity, 1/2.2);

    //intensity += (gl_PrimitiveID % 2) * .05;

    float i = intensity;
    outColor = vec4(i, i, i, 1.0);

    //outColor = vec4(intensity, intensity, intensity, 1.0f);
    //outColor = vec4(normal.x, normal.y, normal.z, 1.0f);
}
