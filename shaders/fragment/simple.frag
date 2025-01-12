#version 450

layout(location = 0) out vec4 outColor;

struct ObjectData
{
	float x[100];
};

layout(set = 0, binding = 0) readonly buffer ObjectBuffer{
	float foo[];
} objectBuffer;

void main()
{
	int x = int(gl_FragCoord.x / 80.0f);
	int y = int(gl_FragCoord.y / 80.0f);
	float value = 0.0f;
	value = objectBuffer.foo[y * 10 + x];

	float fx = float(x);
	float fy = float(y);

	float cx = fx + 0.5f;
	float cy = fy + 0.5f;

	vec2 base = vec2(cos(value), sin(value));
	vec2 dir = vec2((gl_FragCoord.x / 80.f) - cx, (gl_FragCoord.y / 80.f) - cy);

	float d = dot(base, dir);
	
	outColor = vec4(d, d, d, 1.0f);
}
