#version 420 core

layout(location = 0) in vec3 position;

layout(std140, binding = 0) uniform Matrices
{
	mat4 projMatrix;
	mat4 viewMatrix;
};

uniform mat4 modelMatrix;

void main(void) 
{
	gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}