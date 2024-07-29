#version 420 core

uniform float amount;
uniform float power;
uniform vec2 srcResolution;

in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void)
{
	vec2 uv = gl_FragCoord.xy / srcResolution.xy;
	uv *= 1.0 - uv.yx;

	float vig = uv.x * uv.y * amount;
	vig = pow(vig, power);

	fragColour = vec4(vig);
}