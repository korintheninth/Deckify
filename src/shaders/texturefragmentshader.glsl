#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D text;
uniform vec4 textureColor;

void main()
{
	FragColor = texture(text, TexCoord);
}
