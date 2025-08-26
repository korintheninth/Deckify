#version 330 core
layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

uniform vec2 screenSize;

mat4 calculateProjection(float width, float height) {
	return mat4(
		2.0 / width,   0.0,        0.0,        0.0,
		0.0,        2.0 / height,   0.0,        0.0,
		0.0,        0.0,       -1.0,   0.0,
	   -1.0,       -1.0,        0.0,        1.0
	);
}

void main()
{
	mat4 projection = calculateProjection(screenSize.x, screenSize.y);

	gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
	TexCoords = vertex.zw;
}  