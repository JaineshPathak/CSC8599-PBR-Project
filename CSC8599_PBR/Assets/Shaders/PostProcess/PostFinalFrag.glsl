#version 420 core

uniform bool isPostProcessEnabled;

uniform sampler2D srcTexture;
uniform sampler2D postProcessTexture;

layout(std140, binding = 4) uniform u_SkyboxData
{
	vec4 skyboxData;
};

in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void)
{
	float m_GAMMA = skyboxData.y;
	float m_Exposure = skyboxData.x;

	vec3 srcColor = texture(srcTexture, IN.texCoord).rgb;
	vec3 postColor = isPostProcessEnabled ? texture(postProcessTexture, IN.texCoord).rgb : vec3(0.0);

	vec3 result = srcColor + postColor;

	fragColour = vec4(result, 1.0);
}