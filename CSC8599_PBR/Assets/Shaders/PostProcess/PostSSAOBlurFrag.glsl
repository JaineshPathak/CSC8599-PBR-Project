#version 420 core

const float INV_TOTAL_SAMPLES_FACTOR = 1.0 / 16.0;

uniform sampler2D ssaoTexture;

in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void)
{
	vec2 texelSize = 1.0 / vec2(textureSize(ssaoTexture, 0));

	float blurredVisibilityFactor = 0.0;
	for(int t = -2; t < 2; t++)
	{
		for(int s = -2; s < 2; s++)
		{
			vec2 offset = vec2(float(s), float(t)) * texelSize;
			blurredVisibilityFactor += texture(ssaoTexture, IN.texCoord + offset).r;
		}
	}

	fragColour = vec4(blurredVisibilityFactor * INV_TOTAL_SAMPLES_FACTOR);
}