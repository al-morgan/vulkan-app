#version 450

layout(location = 0) out vec4 outColor;

struct ObjectData
{
	float x[100];
};

layout(set = 0, binding = 0) readonly buffer ObjectBuffer{
	float foo[];
} objectBuffer;

#define FACTOR 80.0f

float sample_noise(float x, float y)
{	
	int ix = int(x / FACTOR);
	int iy = int(y / FACTOR);
	float value = objectBuffer.foo[iy * 10 + ix];

	float fx = float(ix);
	float fy = float(iy);

	float cx = fx + 0.5f;
	float cy = fy + 0.5f;

	vec2 base = vec2(cos(value), sin(value));
	vec2 dir = vec2((x / 80.f) - cx, (y / 80.f) - cy);

	float d = dot(base, dir);
	
	return d;
	
}

void main()
{
	float s = sample_noise(gl_FragCoord.x, gl_FragCoord.y);
	outColor = vec4(s, s, s, 1.0f);
}
