#version 150

in vec2 texCoord;
out vec4  fColor;
uniform sampler2D myTexture;

void main()
{
	fColor = texture( myTexture, texCoord );
}

