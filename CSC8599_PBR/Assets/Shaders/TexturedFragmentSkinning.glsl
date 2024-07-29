#version 330 core

uniform float time;
uniform float sinTime;

uniform sampler2D diffuseTex0;
uniform sampler2D diffuseTex1;

float piTimes2 = 3.14 * 2.0;

in Vertex 
{
	vec2 texCoord;
	vec4 weightColor;
} IN;

out vec4 fragColour;

void main(void) 
{
	vec4 texel0 = texture(diffuseTex0, IN.texCoord);
	vec4 texel1 = texture(diffuseTex1, IN.texCoord);

	fragColour = texel0;

	//fragColour =  mix(texel0, texel1, 0.5 * (1.0 + sinTime));
	/*fragColour =  (texture(diffuseTex0, IN.texCoord)) * (texture(diffuseTex1, IN.texCoord) * 2.5f);*/
}