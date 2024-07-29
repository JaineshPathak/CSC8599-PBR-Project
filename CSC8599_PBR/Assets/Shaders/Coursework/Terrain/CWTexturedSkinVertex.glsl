#version 400

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 shadowMatrix;
uniform mat4 lightSpaceMatrix;
uniform int enableFog;

uniform vec3 lightPos;

in vec3 position;
in vec2 texCoord;
in vec3 normal;
in vec4 tangent;
in vec4 jointWeights;
in ivec4 jointIndices;

uniform mat4 joints[128];

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
	vec4 weightColor;
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



	vec4 localPos = vec4(position, 1.0f);
	vec4 skelPos = vec4(0, 0, 0, 0);

	for(int i = 0; i < 4; ++i)
	{
		int jointIndex = jointIndices[i];
		float jointWeight = jointWeights[i];

		skelPos += joints[jointIndex] * localPos * jointWeight;
	}
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(skelPos.xyz, 1.0);
	OUT.texCoord = texCoord;
	OUT.weightColor = vec4(texCoord, 1.0f, 1.0f);



	vec4 worldPos = (modelMatrix * vec4(position, 1.0));
	vec4 posRelativeToCam = viewMatrix * worldPos;
	OUT.worldPos = worldPos.xyz;
	OUT.fragPos = vec3(modelMatrix * vec4(position, 1.0));
	OUT.fragPosLightSpace = lightSpaceMatrix * vec4(OUT.fragPos, 1.0);

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