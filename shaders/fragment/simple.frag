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
	int ix = int(x);
	int iy = int(y);
	float value = objectBuffer.foo[iy * 10 + ix];

	float fx = float(ix);
	float fy = float(iy);

//	float cx = fx + 0.5f;
//	float cy = fy + 0.5f;
//
	float cx = fx;
	float cy = fy ;


	vec2 base = vec2(cos(value), sin(value));
	vec2 dir = vec2(x - cx, y - cy);

	float d = dot(base, dir);
	
	return d;
}

void main()
{
	// grid coordinates
	float gx = gl_FragCoord.x / 80.0f;
	float gy = gl_FragCoord.y / 80.0f;

	// center of sample
	float sx = floor(gx + 0.5f);
	float sy = floor(gy + 0.5f);

	// sample bounds
	float lx = sx - 0.5f;
	float rx = sx + 0.5f;
	float uy = sy - 0.5f;
	float ly = sy + 0.5f;

	// samples
	float ul = sample_noise(lx, uy);
	float ur = sample_noise(rx, uy);
	float ll = sample_noise(lx, ly);
	float lr = sample_noise(rx, ly);

	// interpolation weights
	float wx = gx - lx;
	float wy = gy - uy;

	float x1 = mix(ul, ur, wx);
	float x2 = mix(ll, lr, wx);
	float v = mix(x1, x2, wy);

	//float s = sample_noise(gl_FragCoord.x, gl_FragCoord.y);
	//s = (s + 1.0f) / 2.0f;
	outColor = vec4(v, v, v, 1.0f);
}
