#version 420 core

uniform sampler2D srcTexture;
uniform float brightnessThreshold;
uniform float brightnessSoftThreshold;

in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColour;

vec3 Prefilter(vec3 color)
{
	float brightness = max(color.r, max(color.g, color.b));

	//Soft Knee Calculations
	float k = brightnessThreshold * brightnessSoftThreshold;

	float softK = brightness - brightnessThreshold + k;
	softK = clamp(softK, 0, 2 * k);
	softK = softK * softK / (4.0 * k + 0.000001);

	float contribution = max(softK, brightness - brightnessThreshold);
	contribution /= max(brightness, 0.000001);

	return color * contribution;
}

void main(void)
{
	vec3 color = Prefilter(texture(srcTexture, IN.texCoord).rgb);
	fragColour = vec4(color, 1.0);
}