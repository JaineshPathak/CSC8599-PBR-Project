#version 330 core

uniform sampler2D depthTex;
uniform sampler2D normTex;
uniform sampler2D shadowTex;

uniform vec2 pixelSize;
uniform vec3 cameraPos;

uniform float lightRadius;
uniform vec3 lightPos;
uniform vec4 lightColour;
uniform mat4 inverseProjView;
uniform vec4 shadowMat;

uniform float u_time;
uniform int shouldPulse;

out vec4 diffuseOutput;
out vec4 specularOutput;
out float shadowOutput;

vec4 shadowProj;

void main(void)
{
	vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);
	float depth = texture(depthTex, texCoord.xy).r;

	vec3 ndcPos = vec3(texCoord, depth) * 2.0 - 1.0;
	vec4 invClipPos = inverseProjView * vec4(ndcPos, 1.0);
	vec3 worldPos = invClipPos.xyz / invClipPos.w;

	//lightPos.y = sin(u_time);

	float dist = length(lightPos - worldPos);
	float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);
	if(shouldPulse == 1)
		atten *= (sin(u_time * 3.0f) + 1.0f) / 2.0f;

	if(atten == 0.0)
		discard;

	vec3 normal = normalize(texture(normTex, texCoord.xy).xyz * 2.0 - 1.0);
	vec3 incident = normalize(lightPos - worldPos);
	vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	float lambert = clamp(dot(incident, normal), 0.0, 1.0);
	float rFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
	float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);

	specFactor = pow(specFactor, 60.0f);
	vec3 attenuated = lightColour.xyz * atten;

	//--------------------
	//Shadow

	//vec3 viewDir = normalize(lightPos - worldPos.xyz);
	shadowProj = shadowMat * vec4(worldPos, 1.0);
	float shadow = 1.0;
	vec3 shadowNDC = shadowProj.xyz / shadowProj.w;
	if( abs(shadowNDC.x) < 1.0f && 
		abs(shadowNDC.y) < 1.0f &&
		abs(shadowNDC.z) < 1.0f)
	{
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex, biasCoord.xy).x;
		if(shadowZ < biasCoord.z)
			shadow = 0.0f;
	}

	//--------------------
	
	diffuseOutput = vec4(attenuated * lambert, 1.0);
	specularOutput = vec4(attenuated * specFactor * 0.33, 1.0);
	shadowOutput = shadow;
}