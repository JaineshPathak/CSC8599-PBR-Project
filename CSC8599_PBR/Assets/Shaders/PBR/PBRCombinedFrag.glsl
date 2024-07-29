#version 420 core

uniform sampler2D diffuseTex;

in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void)
{
	vec3 diffuse = texture(diffuseTex, IN.texCoord).rgb;
	fragColour = vec4(diffuse, 1.0);
}