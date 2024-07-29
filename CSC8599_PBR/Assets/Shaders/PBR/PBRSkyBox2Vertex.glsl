#version 420 core

layout(std140, binding = 0) uniform Matrices
{
	mat4 projMatrix;
	mat4 viewMatrix;
};

layout(location = 0) in vec3 position;

out Vertex
{
	vec3 viewDir;
} OUT;

void main(void)
{
	mat4 viewNoTranslation = mat4(mat3(viewMatrix));
	vec4 pos = projMatrix * viewNoTranslation * vec4(position, 1.0);
	gl_Position = pos.xyww;

	OUT.viewDir = position;
}