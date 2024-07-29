#version 420 core

uniform sampler2D mainTex;
uniform vec4 mainColor;

in Vertex 
{
	vec2 texCoord;	
} IN;

out vec4 fragColour;

void main(void)
{
	fragColour = texture(mainTex, IN.texCoord) * mainColor;
}