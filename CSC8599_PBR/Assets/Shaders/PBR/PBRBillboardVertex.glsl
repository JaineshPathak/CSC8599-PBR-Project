#version 420 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

layout(std140, binding = 0) uniform Matrices
{
	mat4 projMatrix;
	mat4 viewMatrix;
};

uniform mat4 billboardMatrix;

out Vertex 
{
	vec2 texCoord;
} OUT;

void main(void) 
{
	mat4 mvp = projMatrix * viewMatrix * billboardMatrix;
	gl_Position = mvp * vec4(position, 1.0);

	OUT.texCoord = texCoord;
}