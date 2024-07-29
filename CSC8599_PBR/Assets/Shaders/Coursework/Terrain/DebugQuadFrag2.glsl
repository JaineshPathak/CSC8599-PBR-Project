#version 330 core

out vec4 fragColour;

in vec2 TexCoords;

uniform sampler2D screenTex;

void main()
{
	vec3 col = texture(screenTex, TexCoords).rgb;
    fragColour = vec4(col, 1.0);
}