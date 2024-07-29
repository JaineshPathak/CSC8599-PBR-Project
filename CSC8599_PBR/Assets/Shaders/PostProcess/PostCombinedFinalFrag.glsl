#version 420 core

uniform sampler2D srcTexture;
uniform sampler2D postTexture;
uniform int combineType = 0;		//0 - Multiply, 1 - Add

in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void)
{
	vec3 srcColor = texture(srcTexture, IN.texCoord).rgb;
	vec3 postColor = texture(postTexture, IN.texCoord).rgb;

	vec3 result = vec3(0.0);
	switch(combineType)
	{
		case 0: result = srcColor * postColor; break;
		case 1: result = srcColor + postColor; break;
	}
	fragColour = vec4(result, 1.0);
}