#version 420 core

in Vertex 
{
	vec3 fragPos;
} IN;

out vec4 fragColour;

void main(void)
{
	fragColour = vec4(IN.fragPos, 1.0);
}