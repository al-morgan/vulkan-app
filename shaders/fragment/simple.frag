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
	// grid coordinates
	float gx = gl_FragCoord.x / 80.0f;
	float gy = gl_FragCoord.y / 80.0f;

	// center of sample
	// float sx = floor(gx + 0.5f);
	// float sy = floor(gy + 0.5f);

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

	// interpolation weights
	//float wx = gx - (floor(gx - 0.5f) + 0.5f);
	//float wy = gy - (floor(gy - 0.5f) + 0.5f);

	float wx = smoothstep(float(alx) + 0.5f, float(alx) + 1.5f, gx);
	float wy = smoothstep(float(aly) + 0.5f, float(aly) + 1.5f, gy);

	float x1 = mix(ul, ur, wx);
	float x2 = mix(ll, lr, wx);
	float v = mix(x1, x2, wy);

	// float x1 = smoothstep(ul, ur, wx);
	// float x2 = smoothstep(ll, lr, wx);
	// float v = smoothstep(x1, x2, wy);

	// v = sample_noise(int(gx), int(gy), gx, gy);

	// v = (v + 1.0f) / 2.0f;

	// v = wy;


	// float s = sample_noise(gl_FragCoord.x, gl_FragCoord.y);
	//s = (s + 1.0f) / 2.0f;
	
	outColor = vec4(max(v, 0.0f), 0.0f, max(-v, 0.0f), 1.0f);

	//v = (v + .707f) / 2.0f;
	
	//outColor = vec4(v, v, v, 1.0f);

	// outColor = vec4((0.7f - max(-v, 0.0f)), max(0.7f - abs(v), 0.0f), (0.7f - max(v, 0.0f)), 1.0f);

	//outColor = vec4(v, v, v, 1.0f);
}
