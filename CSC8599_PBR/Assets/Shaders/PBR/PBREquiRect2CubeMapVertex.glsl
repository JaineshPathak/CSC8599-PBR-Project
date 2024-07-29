#version 420 core

layout(location = 0) in vec3 position;

uniform mat4 proj;
uniform mat4 view;

out Vertex
{
	vec3 localPos;
} OUT;

void main(void)
{
	OUT.localPos = position;
	gl_Position = proj * view * vec4(position, 1.0);
}