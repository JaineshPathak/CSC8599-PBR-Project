#version 330 core

out vec4 fragColour;

in vec2 TexCoords;

uniform sampler2D shadowTex;

void main()
{
	float depthValue = texture(shadowTex, TexCoords).r;
	fragColour = vec4(vec3(depthValue), 1.0);
}