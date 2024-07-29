#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 shadowMatrix;
uniform mat4 lightSpaceMatrix;
uniform int enableFog;

uniform vec3 lightPos;

in vec3 position;
in vec4 colour;
in vec3 normal;
in vec4 tangent;
in vec2 texCoord;
layout (location = 6) in mat4 instanceMatrix;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec3 fragPos;
	vec4 fragPosLightSpace;
	vec4 shadowProj;

	float visibility;
} OUT;

const float density = 0.00015f;
const float gradient = 1.5f;

void main(void)
{
	OUT.colour = colour;
	OUT.texCoord = texCoord;

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

	vec3 wNormal = normalize(normalMatrix * normalize(normal));
	vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

	OUT.normal = wNormal;
	OUT.tangent = wTangent;
	OUT.binormal = cross(wTangent, wNormal) * tangent.w;

	vec4 worldPos = (instanceMatrix * vec4(position, 1));

	vec4 posRelativeToCam = viewMatrix * worldPos;
	OUT.worldPos = worldPos.xyz;
	OUT.fragPos = vec3(modelMatrix * vec4(position, 1.0));
	OUT.fragPosLightSpace = lightSpaceMatrix * vec4(OUT.fragPos, 1.0);

	gl_Position = projMatrix * viewMatrix * worldPos;

	if(enableFog == 1)
	{
		float distance = length(posRelativeToCam.xyz);
		OUT.visibility = exp(-pow((distance * density), gradient));
		OUT.visibility = clamp(OUT.visibility, 0.0, 1.0);
	}

	vec3 viewDir = normalize(lightPos - worldPos.xyz);
	vec4 pushVal = vec4(OUT.normal, 0) * dot(viewDir, OUT.normal);
	OUT.shadowProj = shadowMatrix * (worldPos + pushVal);
}