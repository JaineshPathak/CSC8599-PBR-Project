#version 420 core

//Textures
uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D emissiveTex;
uniform sampler2D occlusionTex;

uniform samplerCube cubeTex;
uniform samplerCube irradianceTex;
uniform sampler2D ssaoTex;
uniform sampler2D shadowTex;

//Flags
uniform bool ssaoEnabled;
uniform bool hasAlbedoTex = false;
uniform bool hasNormalTex = false;
uniform bool hasEmissiveTex = false;
uniform bool hasOcclusionTex = false;

uniform vec3 u_BaseColor = vec3(1.0);
uniform float u_Specular = 1.0;
uniform float u_Shininess = 128.0;
uniform float u_Emission = 1.5;

//Lightings
uniform vec3 cameraPos;

const float GAMMA = 2.2;
const int MAX_POINT_LIGHTS = 100;
const int MAX_SPOT_LIGHTS = 2;
const float PI = 3.14159265359;

struct PointLight
{
	vec4 lightPosition;
	vec4 lightColor;
	vec4 lightAttenData;
};

struct DirectionalLight
{
	vec4 lightDirection;
	vec4 lightColor;
};

struct SpotLight
{
	vec4 lightPosition;
	vec4 lightDirection;
	vec4 lightColor;
	vec4 lightAttenData;
	vec4 lightCutoffData;
};

layout(std140, binding = 1) uniform u_PointLights
{
	int numPointLights;
	PointLight pointLights[MAX_POINT_LIGHTS];
};

layout(std140, binding = 2) uniform u_DirectionLight
{
	DirectionalLight directionalLight;
};

layout(std140, binding = 3) uniform u_SpotLights
{
	int numSpotLights;
	SpotLight spotLights[MAX_SPOT_LIGHTS];
};

layout(std140, binding = 4) uniform u_SkyboxData
{
	vec4 skyboxData;
};

in Vertex 
{
	vec3 position;
	vec2 texCoord;
	vec3 normal;
	vec3 fragWorldPos;
	vec4 fragClipSpacePos;
	vec4 fragLightSpacePos;
	vec3 tangent;
	vec3 bitangent;
	mat3 TBN;
} IN;

out vec4 fragColour;

float CalcShadows(float NdotL)
{
	vec3 projCoords = IN.fragLightSpacePos.xyz / IN.fragLightSpacePos.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowTex, projCoords.xy).r;
	float currentDepth = projCoords.z;

	float bias = max(0.01 * (1.0 - NdotL), 0.005);
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowTex, 0);
	const int halfkernelWidth = 3;
	for(int x = -halfkernelWidth; x <= halfkernelWidth; x++)
	{
		for(int y = -halfkernelWidth; y <= halfkernelWidth; y++)
		{
			float pcfDepth = texture(shadowTex, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	
	//shadow /= 9.0;
	shadow /= ((halfkernelWidth * 2.0 + 1.0) * (halfkernelWidth * 2.0 + 1.0));

	if(projCoords.z > 1.0)
		shadow = 1.0;

	return shadow;
}

void CalcDirectionalLight(inout vec3 result, in vec3 albedoColor, in vec3 normalColor)
{
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * vec3(directionalLight.lightColor.xyz);

	vec3 norm = normalize(normalColor);
	vec3 lightDir = normalize(-directionalLight.lightDirection.xyz);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(directionalLight.lightColor.xyz);

	vec3 viewDir = normalize(cameraPos - IN.fragWorldPos);
	vec3 viewDirHalf = normalize(lightDir + viewDir);
	//vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(norm, viewDirHalf), 0.0), u_Shininess);
	vec3 specular = u_Specular * spec * vec3(directionalLight.lightColor.xyz);

	float NdotL = max(dot(normalColor, lightDir), 0.0001);
	float shadow = CalcShadows(NdotL);

	result = (1.0 / PI) * (diffuse + specular) * (1.0 - shadow) * albedoColor;
}

void CalcPointsLights(inout vec3 result, in vec3 albedoColor, in vec3 normalColor)
{
	for(int i = 0; i < numPointLights; i++)
	{
		float ambientStrength = 0.3;
		vec3 ambient = ambientStrength * vec3(pointLights[i].lightColor.xyz);

		vec3 norm = normalize(normalColor);
		vec3 lightDir = normalize(pointLights[i].lightPosition.xyz - IN.fragWorldPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * vec3(pointLights[i].lightColor.xyz);

		vec3 viewDir = normalize(cameraPos - IN.fragWorldPos);
		vec3 viewDirHalf = normalize(lightDir + viewDir);
		//vec3 reflectDir = reflect(-lightDir, norm);

		float spec = pow(max(dot(norm, viewDirHalf), 0.0), u_Shininess);
		vec3 specular = u_Specular * spec * vec3(pointLights[i].lightColor.xyz);

		float distance = length(pointLights[i].lightPosition.xyz - IN.fragWorldPos);
		//float attenuation = 1.0 / (pointLights[i].lightAttenData.x + pointLights[i].lightAttenData.y * distance + pointLights[i].lightAttenData.z * (distance * distance));
		float attenuation = 1.0 / distance * distance;

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;

		result += (1.0 / PI) * (diffuse + specular) * albedoColor;		
	}
}

void CalcSpotLights(inout vec3 result, in vec3 albedoColor, in vec3 normalColor)
{
	for(int i = 0; i < numSpotLights; i++)
	{
		vec3 lightDir = normalize(spotLights[i].lightPosition.xyz - IN.fragWorldPos);
		float theta = dot(lightDir, normalize(-spotLights[i].lightDirection.xyz));
		float epsilon = spotLights[i].lightCutoffData.x - spotLights[i].lightCutoffData.y;
		float edgeFactor = clamp((theta - spotLights[i].lightCutoffData.y) / epsilon, 0.0, 1.0);

		float ambientStrength = 0.3;
		vec3 ambient = ambientStrength * vec3(spotLights[i].lightColor.xyz);
			
		vec3 norm = normalize(normalColor);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * vec3(spotLights[i].lightColor.xyz);

		vec3 viewDir = normalize(cameraPos - IN.fragWorldPos);
		vec3 viewDirHalf = normalize(lightDir + viewDir);
		//vec3 reflectDir = reflect(-lightDir, norm);

		float spec = pow(max(dot(norm, viewDirHalf), 0.0), u_Shininess);
		vec3 specular = u_Specular * spec * vec3(spotLights[i].lightColor.xyz);

		float distance = length(spotLights[i].lightPosition.xyz - IN.fragWorldPos);
		//float attenuation = 1.0 / (spotLights[i].lightAttenData.x + spotLights[i].lightAttenData.y * distance + spotLights[i].lightAttenData.z * (distance * distance));
		float attenuation = 1.0 / distance * distance;

		ambient *= attenuation * edgeFactor;
		diffuse *= attenuation * edgeFactor;
		specular *= attenuation * edgeFactor;

		result += (1.0 / PI) * (diffuse + specular) * albedoColor;		
	}
}

void CalcAmbientLight(inout vec3 result, in vec3 albedoColor, in vec3 normalColor)
{
	vec3 irradiance = texture(irradianceTex, normalColor).rgb;
	vec3 diffuse = irradiance * albedoColor;

	float ambientStrength = 0.5;
	vec3 ambient = ambientStrength * diffuse;

	//SSAO
	float aoStrength = 1.0;
	if(ssaoEnabled)
	{
		vec2 NDCSpaceFragPos = IN.fragClipSpacePos.xy / IN.fragClipSpacePos.w;
		vec2 texLoopUps = NDCSpaceFragPos * 0.5 + 0.5;
		aoStrength = texture(ssaoTex, texLoopUps).r;
	}

	float aoTexStrength = hasOcclusionTex ? texture(occlusionTex, IN.texCoord).r : 1.0;
	ambient *= aoTexStrength;
	ambient *= aoStrength;

	result += ambient;
}

void CalcReflection(inout vec3 result, in vec3 albedoColor, in vec3 normalColor)
{
	vec3 I = normalize(IN.position - cameraPos);
	vec3 R = reflect(I, normalize(normalColor));
	result += (1.0 / PI) * texture(cubeTex, R).rgb * albedoColor;
}

void CalcRefraction(inout vec3 result, in vec3 albedoColor, in vec3 normalColor)
{
	float refractiveIndex = 1.00 / 1.5848;	//Metal - Iron
	vec3 I = normalize(IN.position - cameraPos);
	vec3 R = refract(I, normalize(normalColor), refractiveIndex);
	result += (1.0 / PI) * texture(cubeTex, -R).rgb * albedoColor;
}

void main(void) 
{
	float m_GAMMA = skyboxData.y;
	float m_Exposure = skyboxData.x;

	vec3 albedoColor = hasAlbedoTex ? texture(albedoTex, IN.texCoord).rgb * u_BaseColor : u_BaseColor;
	albedoColor = pow(albedoColor, vec3(m_GAMMA));

	mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.bitangent), normalize(IN.normal));
	vec3 normalColor = IN.normal;
	if(hasNormalTex)
	{
		normalColor = texture(normalTex, IN.texCoord).rgb;
		normalColor = normalColor * 2.0 - 1.0;
		normalColor.xy *= 1.0;
		//normalColor = normalize(IN.TBN * normalColor);
	}
	normalColor = normalize(TBN * normalize(normalColor));

	vec3 result = vec3(0.0);
	CalcDirectionalLight(result, albedoColor, normalColor);	
	CalcPointsLights(result, albedoColor, normalColor);
	CalcSpotLights(result, albedoColor, normalColor);
	CalcRefraction(result, albedoColor, normalColor);	
	CalcAmbientLight(result, albedoColor, normalColor);

	//CalcReflection(result, albedoColor, normalColor);

	//Gamma	
	//result = vec3(1.0) - exp(-result * 1.5);
	//result = pow(result, vec3(1.0 / GAMMA));
	//result = result / (vec3(0.5) + result);
	//result = reinhardSimple(result);
	//result = reinhardExtended(result, 1.0f);

	result = vec3(1.0) - exp(-result * m_Exposure);
	result = pow(result, vec3(1.0 / m_GAMMA));

	if(hasEmissiveTex)
	{
		vec3 emissiveColor = texture(emissiveTex, IN.texCoord).rgb;
		result += emissiveColor * u_Emission;
	}

	fragColour = vec4(result, 1.0);
}