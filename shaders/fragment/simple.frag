#version 450


layout (push_constant) uniform my_push_constants_t
{
	float x;
	float y;
	float zoom;
} my_push_constants;

layout(location = 0) out vec4 outColor;

int MAX_INTENSITY_INT = 100;
float MAX_INTENSITY_FLOAT = float(MAX_INTENSITY_INT);

void main()
{
	float a = (gl_FragCoord.x / 200.0f - 2.0f) / my_push_constants.zoom + my_push_constants.x;
	float b = (gl_FragCoord.y / 200.0f - 2.0f) / my_push_constants.zoom + my_push_constants.y;

	vec4 color;

	float z1 = 0.0f;
	float z2 = 0.0f;

	int i;

	color = vec4(0.0, 0.0, 0.0, 0.0);

	for(i = 0; i < MAX_INTENSITY_INT; i++)
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
		float remapped = f * 320.0f / MAX_INTENSITY_FLOAT + 390.f;
		//remapped = 690.f - remapped;

		float red = max(100.0f - abs(remapped - 575.0f), 0.0f);
		float green = max(100.0f - abs(remapped - 540.0f), 0.0f);
		float blue = max(50.0f - abs(remapped - 440.0f), 0.0f);

		red /= 100.0f;
		green /= 100.0f;
		blue /= 50.0f;

		color = vec4(red, green, blue, 1.0);
	}

	if(i >= 100)
	{
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}

	outColor = color;

}
