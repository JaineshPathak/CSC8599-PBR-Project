#version 420 core

in Vertex
{
	vec3 localPos;
} IN;

uniform sampler2D equiRectTex;

const float GAMMA = 2.2;
const vec2 invAtan = vec2(0.1591, 0.3183);

out vec4 fragColour;

vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main(void)
{
	vec2 uv = SampleSphericalMap(normalize(IN.localPos));
	
	//Tone Mapping and Gamma Correction
	vec3 finalColor = texture(equiRectTex, uv).rgb;
	finalColor = pow(finalColor, vec3(GAMMA));
	finalColor = finalColor / (finalColor + vec3(1.0));
	finalColor = pow(finalColor, vec3(1.0 / GAMMA));
	
	fragColour = vec4(finalColor, 1.0);
}