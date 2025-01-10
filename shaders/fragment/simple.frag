#version 450

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(gl_FragCoord.x / 800.0f, gl_FragCoord.y / 600.0f , 1.0, 1.0);
}
