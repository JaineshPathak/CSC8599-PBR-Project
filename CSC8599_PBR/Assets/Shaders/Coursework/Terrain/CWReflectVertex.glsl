#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;
uniform int enableFog;

in vec3 position;
in vec3 normal;
in vec2 texCoord;
in vec4 tangent;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;

	float visibility;
} OUT;

const float density = 0.00015f;
const float gradient = 1.5f;

void main(void)
{
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	

	vec3 wNormal = normalize(normalMatrix * normalize(normal));
	vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

	OUT.normal = wNormal;
	OUT.tangent = wTangent;
	OUT.binormal = cross(wTangent, wNormal) * tangent.w;



	OUT.texCoord = (textureMatrix * vec4(texCoord, 0.0, 1.0)).xy;
	OUT.normal = normalize(normalMatrix * normalize(normal));

	vec4 worldPos = (modelMatrix * vec4(position, 1));
	OUT.worldPos = worldPos.xyz;

	gl_Position = (projMatrix * viewMatrix) * worldPos;

	vec4 posRelativeToCam = viewMatrix * worldPos;
	if(enableFog == 1)
	{
		float distance = length(posRelativeToCam.xyz);
		OUT.visibility = exp(-pow((distance * density), gradient));
		OUT.visibility = clamp(OUT.visibility, 0.0, 1.0);
	}
}