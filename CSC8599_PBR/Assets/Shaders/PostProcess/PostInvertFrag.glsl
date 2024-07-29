#version 420 core

uniform sampler2D srcTexture;

in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void)
{
	vec3 srcColor = texture(srcTexture, IN.texCoord).rgb;
	vec3 result = vec3(1.0) - srcColor;

	fragColour = vec4(result, 1.0);
}