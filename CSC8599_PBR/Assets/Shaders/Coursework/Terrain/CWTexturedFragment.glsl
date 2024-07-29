#version 330 core

uniform sampler2D diffuseTex;

in Vertex 
{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void) 
{
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	fragColour = diffuse;
	fragColour.a = diffuse.a;
}