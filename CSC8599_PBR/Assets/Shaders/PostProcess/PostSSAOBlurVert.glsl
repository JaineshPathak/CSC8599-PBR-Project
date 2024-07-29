#version 420 core

layout(location = 0) in vec3 position;

out Vertex
{
	vec2 texCoord;
} OUT;

void main(void)
{
	gl_Position = vec4(position, 1.0);
	OUT.texCoord = position.xy * 0.5 + 0.5;
}