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
	vec3 pos = position;
	mat4 invProj = inverse(projMatrix);

	pos.xy *= vec2(invProj[0][0], invProj[1][1]);
	pos.z = -1.0f;

	OUT.viewDir = transpose(mat3(viewMatrix)) * normalize(pos);
	gl_Position = vec4(position, 1.0);
}