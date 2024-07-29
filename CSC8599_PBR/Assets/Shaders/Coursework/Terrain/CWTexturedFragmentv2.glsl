#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D shadowTex;

uniform vec3 cameraPos;

//Directional Light
uniform int flipLightDir;
uniform vec3 lightDir;
uniform vec4 lightDirColour;
uniform float lightDirIntensity;

//Point Light
uniform int numPointLights;
uniform vec3 pointLightPos[50];
uniform vec4 pointLightColour[50];
uniform vec4 pointLightSpecularColour[50];
uniform float pointLightRadius[50];
uniform float pointLightIntensity[50];

uniform vec4 lightColour;
uniform vec4 specularColour;
uniform vec3 lightPos;
uniform float lightRadius;

uniform int enableFog;
uniform vec4 fogColour;

uniform float u_time;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec3 fragPos;
	vec4 fragPosLightSpace;
	vec4 shadowProj;

	float visibility;
} IN;

out vec4 fragColour;
out vec4 brightColour;

vec3 CalcDirLight(vec3 viewDir, vec3 bumpNormal);
vec3 CalcPointLight(vec4 _pointLightColour, vec4 _pointLightSpecularColour, vec3 _pointLightPos, float _pointLightRadius, float _pointLightIntensity, vec3 _viewDir, vec3 _bumpNormal);

void main(void)
{
	//==============================================================================================

	float diffuseAlpha = texture(diffuseTex, IN.texCoord).a;
	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	vec3 bumpNormal = texture(bumpTex, IN.texCoord ).xyz;
	bumpNormal = bumpNormal * 2.0 - 1.0;
	bumpNormal.xy *= 1.0;
	bumpNormal = normalize(bumpNormal);

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	bumpNormal = normalize(TBN * bumpNormal);

	vec3 result = vec3(0.0);
	result += CalcDirLight(viewDir, bumpNormal);
	if(numPointLights > 0)
	{
		for(int i = 0; i < numPointLights; i++)
			result += CalcPointLight(pointLightColour[i], pointLightSpecularColour[i], pointLightPos[i], pointLightRadius[i], pointLightIntensity[i], viewDir, bumpNormal);
	}

	fragColour = vec4(result, diffuseAlpha);
	if(enableFog == 1)
	{
		fragColour = mix(vec4(fogColour.xyz, diffuseAlpha), fragColour, IN.visibility);
	}

	float brightness = dot(fragColour.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.75)
        brightColour = vec4(fragColour.rgb, diffuseAlpha);
    else
        brightColour = vec4(0.0, 0.0, 0.0, diffuseAlpha);
	//fragColour = vec4(1.0);
}

vec3 CalcDirLight(vec3 viewDir, vec3 bumpNormal)
{
	vec3 incident = normalize(lightDir);
	vec3 halfDir = normalize(incident + viewDir);	

	float lambert = max(dot(incident, bumpNormal), 0.0);
	float attenuation = 1.0f;

	float specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0f);

	vec3 ambient = 0.1f * texture(diffuseTex, IN.texCoord).rgb;
	vec3 diffuseRGB = lightDirColour.rgb * texture(diffuseTex, IN.texCoord).rgb * lambert;
	vec3 specular = lightDirColour.rgb * (specularColour.rgb * specFactor);

	//--------------------
	//Shadow

//	float shadow = 1.0;
//	vec3 shadowNDC = IN.shadowProj.xyz / IN.shadowProj.w;
//	if( abs(shadowNDC.x) < 1.0f && 
//		abs(shadowNDC.y) < 1.0f &&
//		abs(shadowNDC.z) < 1.0f)
//	{
//		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
//		float shadowZ = texture(shadowTex, biasCoord.xy).x;
//		if(shadowZ < biasCoord.z)
//			shadow = 0.5f;
//	}

//	float shadow = 1.0;
//	vec3 projCoords = IN.shadowProj.xyz / IN.shadowProj.w;
//	vec2 UVCoords;
//	UVCoords.x = 0.5 * projCoords.x + 0.5;
//	UVCoords.y = 0.5 * projCoords.y + 0.5;
//	float z = 0.5 * projCoords.z + 0.5;
//
//	float depth = texture(shadowTex, UVCoords).x;
//	float bias = 0.0025;
//
//	if(depth + bias < z)
//		shadow = 0.6;

	float shadow = 0.0;
	vec3 projCoords = IN.fragPosLightSpace.xyz / IN.fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowTex, projCoords.xy).r;
	float currentDepth = projCoords.z;
	vec3 normal = normalize(IN.normal);
    vec3 lighterDir = normalize(lightPos - IN.fragPos);
	float bias = max(0.05 * (1.0 - dot(normal, lighterDir)), 0.005);

	shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	//--------------------

	//ambient *= shadow;
	//diffuseRGB *= (1.0 - (shadow * 0.7));
	//specular *= (1.0 - (shadow * 0.7));

	return (ambient + diffuseRGB + specular) * lightDirIntensity;
}

vec3 CalcPointLight(vec4 _pointLightColour, vec4 _pointLightSpecularColour, vec3 _pointLightPos, float _pointLightRadius, float _pointLightIntensity, vec3 _viewDir, vec3 _bumpNormal)
{
	vec3 incident = normalize(_pointLightPos - IN.worldPos);
	vec3 halfDir = normalize(incident + _viewDir);

	float lambert = max(dot(incident, _bumpNormal), 0.0);
	float distance = length(_pointLightPos - IN.worldPos);
	float attenuation = 1.0 - clamp( (distance / _pointLightRadius), 0.0, 1.0);

	float specFactor = clamp(dot(halfDir, _bumpNormal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0f);

	vec3 ambient = 0.1f * texture(diffuseTex, IN.texCoord).rgb;
	vec3 diffuseRGB = _pointLightColour.rgb * texture(diffuseTex, IN.texCoord).rgb * lambert;
	vec3 specular = _pointLightColour.rgb * (_pointLightSpecularColour.rgb * specFactor);

	//--------------------
	//Shadow

//	float shadow = 1.0;
//	vec3 projCoords = IN.shadowProj.xyz / IN.shadowProj.w;
//	vec2 UVCoords;
//	UVCoords.x = 0.5 * projCoords.x + 0.5;
//	UVCoords.y = 0.5 * projCoords.y + 0.5;
//	float z = 0.5 * projCoords.z + 0.5;
//
//	float depth = texture(shadowTex, UVCoords).x;
//	float bias = 0.0025;
//
//	if(depth + bias < z)
//		shadow = 0.0;

	float shadow = 0.0;
	vec3 projCoords = IN.shadowProj.xyz / IN.shadowProj.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowTex, projCoords.xy).r;
	float currentDepth = projCoords.z;
	vec3 normal = normalize(IN.normal);
    vec3 lightDir = normalize(_pointLightPos - IN.fragPos);
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

	shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	//--------------------

	ambient *= attenuation * _pointLightIntensity;
	//ambient *= shadow;

	diffuseRGB *= attenuation * _pointLightIntensity;
	//diffuseRGB *= shadow;

	specular *= attenuation * _pointLightIntensity;
	//specular *= shadow;

	return (ambient + diffuseRGB + specular);
}