#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) in vec2 coords;



struct ObjectData
{
	float x[100];
};

layout(set = 0, binding = 0) readonly buffer ObjectBuffer{
	float foo[];
} objectBuffer;

layout(set = 0, binding = 2) readonly buffer NormalBuffer{
	vec4 bar[];
} normalBuffer;

layout(binding = 1) uniform uniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

#define FACTOR 80.0f

float sample_noise(int ax, int ay, float gx, float gy)
{	
	float value = objectBuffer.foo[ay * 10 + ax];

	float cx = float(ax) + 0.5f;
	float cy = float(ay) + 0.5f;

	vec2 base = vec2(cos(value), sin(value));
	vec2 dir = vec2(gx - cx, gy - cy);

	float d = dot(base, dir);
	
	return d;
}

void main()
{
	
	float gx = coords.x * 5.0f + 5.0f;
	float gy = coords.y * 5.0f + 5.0f;

	

	// sample bounds
	float lx = gx - 0.5f;
	float rx = gx + 0.5f;
	float uy = gy - 0.5f;
	float ly = gy + 0.5f;

	int alx = int(lx);
	int aly = int(uy);

	// samples
	float ul = sample_noise(alx, aly, gx, gy);
	float ur = sample_noise(alx + 1, aly, gx, gy);
	float ll = sample_noise(alx, aly + 1, gx, gy);
	float lr = sample_noise(alx + 1, aly + 1, gx, gy);

	float wx = smoothstep(float(alx) + 0.5f, float(alx) + 1.5f, gx);
	float wy = smoothstep(float(aly) + 0.5f, float(aly) + 1.5f, gy);

	float x1 = mix(ul, ur, wx);
	float x2 = mix(ll, lr, wx);
	float v = mix(x1, x2, wy);
	

	vec4 n = normalBuffer.bar[gl_PrimitiveID];
	vec3 light = vec3(.9f, 0.0, .107f);
	
	float d = dot(vec3(n.x, n.y, n.z), light);

    //d = n.z;

    //d = gl_PrimitiveID / 2000000.0f;

	//float d = (dot(vec3(n.x, n.y, n.z), light) - .5f) * 2.0f;

	//float c = gl_PrimitiveID / 1000000.0f;

	//outColor = vec4(n.r, n.g, n.b, 1.0f);
	//outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	outColor = vec4(d, d, d, 1.0f);

	//outColor = vec4(c, c, c, 1.0f);

	// GOOD ONE HERE
	//outColor = vec4(max(v, 0.0f), 0.0f, max(-v, 0.0f), 1.0f);
}
