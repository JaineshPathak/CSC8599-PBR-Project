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
uniform mat4 lightSpaceMatrix;

out Vertex 
{
	vec3 position;
	vec2 texCoord;
	vec3 normal;
	vec3 fragWorldPos;
	vec4 fragClipSpacePos;
	vec4 fragLightSpacePos;
	vec3 tangent;
	vec3 bitangent;
	mat3 TBN;
} OUT;

void main(void) 
{
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(position, 1.0);

	mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
	vec3 oNormal = normalize(normalMatrix * normalize(normal));
	vec3 oTangent = normalize(normalMatrix * normalize(tangent.xyz));
	vec3 oBiTangent = cross(oTangent, oNormal) * tangent.w;
	mat3 oTBN = mat3(normalize(oNormal), normalize(oBiTangent), normalize(oNormal));

	OUT.position = vec3(modelMatrix * vec4(position, 1.0));
	OUT.texCoord = vec2(texCoord.x, texCoord.y);
	OUT.normal = oNormal;
	OUT.fragWorldPos = vec3(modelMatrix * vec4(position, 1.0));
	OUT.fragClipSpacePos = gl_Position;
	OUT.fragLightSpacePos = lightSpaceMatrix * vec4(position, 1.0);
	OUT.tangent = oTangent;
	OUT.bitangent = oBiTangent;
	OUT.TBN = oTBN;
}