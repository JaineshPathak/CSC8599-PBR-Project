#version 420 core

layout(location = 0) in vec3 position;

uniform mat4 lightSpaceMatrix;
uniform mat4 modelMatrix;

void main(void) 
{
	gl_Position = lightSpaceMatrix * modelMatrix * vec4(position, 1.0);
}