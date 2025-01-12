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

	if(value >= 1.0f)
	{
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		outColor = vec4(0.0, 1.0, 0.0, 1.0);
	}	
}
