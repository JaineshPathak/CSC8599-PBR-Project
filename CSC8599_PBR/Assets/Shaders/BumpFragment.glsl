#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;

uniform vec3 cameraPos;

uniform int lightType;
uniform vec3 lightDir;

uniform vec4 lightColour;
uniform vec4 specularColour;
uniform vec3 lightPos;
uniform float lightRadius;

uniform float u_time;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

void main(void)
{
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	vec3 bumpNormal = texture(bumpTex, IN.texCoord ).xyz;
	bumpNormal = bumpNormal * 2.0 - 1.0;
	bumpNormal.xy *= 0.8;
	bumpNormal = normalize(bumpNormal);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	vec3 incident;
	incident = lightType == 0 ? normalize(lightPos - IN.worldPos) normalize(lightDir);
	vec3 halfDir = normalize(incident + viewDir);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	bumpNormal = normalize(TBN * bumpNormal);


	float lambert = max(dot(incident, bumpNormal), 0.0);
	float distance = length(lightPos - IN.worldPos);
	float attenuation;
	attenuation = lightType == 0 ? 1.0 - clamp( (distance / lightRadius), 0.0, 1.0) : 1.0;
	//attenuation *= (sin(u_time * 2.0) + 1.0) / 2.0;

	float specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0f);

	vec3 surface = (diffuse.rgb * lightColour.rgb);
	fragColour.rgb = surface * lambert * attenuation;
	fragColour.rgb += (lightColour.rgb * (specularColour.rgb * specFactor)) * attenuation;
	fragColour.rgb += surface * 0.1f;			//Ambient
	fragColour.a = diffuse.a;					//Copies Alpha
}