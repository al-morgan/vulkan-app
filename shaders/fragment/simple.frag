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
	float x = (gl_FragCoord.x / 800.0f) * 10.f;
	float y = (gl_FragCoord.y / 800.0f) * 10.f;
	float value = 0.0f;
	
	value = objectBuffer.foo[int(y * 10 + x)];

	if(value >= 1.0f)
	{
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		outColor = vec4(0.0, 1.0, 0.0, 1.0);
	}	
}
