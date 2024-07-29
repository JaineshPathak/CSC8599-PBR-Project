#version 420 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 tangent;

layout(std140, binding = 0) uniform Matrices
{
	mat4 projMatrix;
	mat4 viewMatrix;
};

uniform mat4 modelMatrix;

out Vertex 
{
	vec3 fragPos;
} OUT;

void main(void) 
{
	vec4 viewPos = viewMatrix * modelMatrix * vec4(position, 1.0);
	gl_Position = projMatrix * viewPos;

	OUT.fragPos = viewPos.xyz;
}