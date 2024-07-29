#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform mat4 lightSpaceMatrix = mat4(1.0f);

in vec3 position;

void main(void)
{
	//gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(position, 1.0f);
	gl_Position = lightSpaceMatrix * vec4(position, 1.0);
}