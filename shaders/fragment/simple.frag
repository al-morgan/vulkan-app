#version 450


layout (push_constant) uniform my_push_constants_t
{
	float x;
	float y;
	float zoom;
} my_push_constants;

layout(location = 0) out vec4 outColor;

void main()
{
	float a = (gl_FragCoord.x / 200.0f - 2.0f) * my_push_constants.zoom;
	float b = (gl_FragCoord.y / 200.0f - 2.0f) * my_push_constants.zoom;

	vec4 color;

	float z1 = 0.0f;
	float z2 = 0.0f;

	int i;

	color = vec4(0.0, 0.0, 0.0, 0.0);

	for(i = 0; i < 100; i++)
	{
		// square z
		float r1 = z1 * z1;		// first
		float r2 = 2 * z1 * z2;	// inside-outside
		float r3 = -(z2 * z2);	// last (i squared is negative one)

		z1 = r1 + r3 + a;
		z2 = r2 + b;
		
		float mag = sqrt(z1 * z1 + z2 * z2);
		
		if(mag > 2.0)
		{
			break;
		}

		//color = vec4(1.0, 1.0, 1.0, 1.0);
		float f = float(i);
		color = vec4(f / 100.0f, f / 100.0f, f / 100.0f, 1.0);
	}

	if(i >= 100)
	{
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}

	outColor = color;

}
