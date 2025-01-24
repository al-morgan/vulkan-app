#version 450
#extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 coords;

layout(set = 0, binding = 0) readonly buffer ObjectBuffer{
	float foo[];
} objectBuffer;

layout(binding = 1) uniform uniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

//
// vec2 positions[3] = vec2[]
// (
// 	vec2(0.0,-0.5),
// 	vec2(0.5, 0.5),
// 	vec2(-0.5, 0.5)
// );

//#define MIN -1.0
//#define MAX 1.0

//vec2 positions[6] = vec2[]
//(
//	vec2(MIN, MIN),
//	vec2(MAX, MIN),
//	vec2(MIN, MAX),
//	vec2(MAX, MIN),
//	vec2(MAX, MAX),
//	vec2(MIN, MAX)
//); 

vec3 colors[3] = vec3[]
(
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

void main()
{
	vec4 foo = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	fragColor = vec4(foo.x, foo.y, foo.z, 1.0);
	gl_Position = foo;
	coords = vec2(inPosition.x, inPosition.y);

	//gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);	
	
	//gl_Position = vec4(inPosition, 0.0, 1.0);
	//fragColor = colors[gl_VertexIndex];
}
