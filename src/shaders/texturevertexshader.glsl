#version 330 core
layout (location = 0) in vec4 vertex;
out vec2 TexCoord;
uniform vec2 screenSize;

void main()
{
	vec2 ndc = vertex.xy / screenSize * 2.0 - 1.0;
	gl_Position = vec4(ndc, 0.0, 1.0);

	TexCoord = vertex.zw;
}
