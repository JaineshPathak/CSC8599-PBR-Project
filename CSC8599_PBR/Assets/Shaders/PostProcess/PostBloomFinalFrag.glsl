#version 420 core

//uniform sampler2D srcTexture;
uniform sampler2D postProcessTexture;
uniform sampler2D dirtMaskTexture;

uniform float bloomStrength;

uniform vec4 bloomTint;
uniform float bloomTintStrength;

uniform bool enableDirtMask;
uniform float dirtMaskStrength;

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

	//vec3 srcColor = texture(srcTexture, IN.texCoord).rgb;
	vec3 postColor = texture(postProcessTexture, IN.texCoord).rgb;
	vec3 dirtColor = texture(dirtMaskTexture, IN.texCoord).rgb * dirtMaskStrength;
	vec3 tintColor = postColor + postColor * bloomTint.rgb * bloomTintStrength;

	if(enableDirtMask)
		postColor = postColor + postColor * dirtColor;

	vec3 result = (postColor + tintColor) * bloomStrength;

	//result = pow(result, vec3(m_GAMMA));

	//result = vec3(1.0) - exp(-result * m_Exposure);
	//result = pow(result, vec3(1.0 / m_GAMMA));

    fragColour = vec4(result, 1.0);
}