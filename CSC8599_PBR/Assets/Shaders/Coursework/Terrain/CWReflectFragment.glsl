#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform samplerCube cubeTex;

uniform vec3 cameraPos;
uniform int enableFog;
uniform vec4 fogColour;


//Directional Light
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



in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;

	float visibility;
} IN;

out vec4 fragColour;

uniform float u_time;

vec3 CalcDirLight(vec3 viewDir, vec3 bumpNormal);
vec3 CalcPointLight(vec4 _pointLightColour, vec4 _pointLightSpecularColour, vec3 _pointLightPos, float _pointLightRadius, float _pointLightIntensity, vec3 _viewDir, vec3 _bumpNormal);

void main(void)
{
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);

	vec3 reflectDir = reflect(-viewDir, normalize(IN.normal));
	vec4 reflectTex = texture(cubeTex, reflectDir);


	vec3 bumpNormal = texture(bumpTex, IN.texCoord ).xyz;
	bumpNormal = bumpNormal * 2.0 - 1.0;
	bumpNormal.xy *= 5.0;
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


	//fragColour = reflectTex + (diffuse * 0.25f);
	fragColour = (reflectTex * vec4(result, 1.0f));
	if(enableFog == 1)
	{
		fragColour = mix(fogColour, fragColour, IN.visibility);
	}
	fragColour.a = 0.9f;
}

vec3 CalcDirLight(vec3 viewDir, vec3 bumpNormal)
{
	vec3 incident = normalize(lightDir);
	vec3 halfDir = normalize(incident + viewDir);	

	float lambert = max(dot(incident, bumpNormal), 0.0);
	float attenuation = 1.0f;

	float specFactor = clamp(dot(halfDir, bumpNormal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0f);

	vec3 ambient = 0.3f * texture(diffuseTex, IN.texCoord).rgb;
	vec3 diffuseRGB = lightDirColour.rgb * texture(diffuseTex, IN.texCoord).rgb * lambert;
	vec3 specular = lightDirColour.rgb * (specularColour.rgb * specFactor);

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

	ambient *= attenuation * _pointLightIntensity;
	diffuseRGB *= attenuation * _pointLightIntensity;
	specular *= attenuation * _pointLightIntensity;

	return (ambient + diffuseRGB + specular);
}