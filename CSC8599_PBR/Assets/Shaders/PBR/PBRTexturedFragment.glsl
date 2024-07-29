#version 420 core

//Textures
uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D metallicTex;
uniform sampler2D roughnessTex;
uniform sampler2D emissiveTex;
uniform sampler2D occlusionTex;

uniform samplerCube irradianceTex;
uniform samplerCube prefilterTex;
uniform sampler2D brdfLUTTex;
uniform sampler2D ssaoTex;
uniform sampler2D shadowTex;

//Flags
uniform bool ssaoEnabled;
uniform bool hasAlbedoTex = false;
uniform bool hasNormalTex = false;
uniform bool hasMetallicTex = false;
uniform bool hasRoughnessTex = false;
uniform bool hasEmissiveTex = false;
uniform bool hasOcclusionTex = false;

uniform vec3 u_BaseColor = vec3(1.0);
uniform float u_Metallic = 0.01;
uniform float u_Roughness = 0.5;
uniform float u_Emission = 1.5;

//Lightings
uniform vec3 cameraPos;

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

float DistributionGGX(vec3 N, vec3 H, float roughnessStrength)
{
	float numerator = roughnessStrength * roughnessStrength;
	
	float NdotH = max(dot(N, H), 0.0);
	float NdotHSq = NdotH * NdotH;

	float denominator = (NdotHSq * (numerator - 1.0) + 1.0);
	denominator = PI * denominator * denominator;
	
	return numerator / denominator;
}

float GeometrySchlickGGX(float dot, float roughnessStrength)
{
	float numerator = dot;

	float a = roughnessStrength + 1;
	float k = (a * a) / 8.0;

	float denominator = dot * (1.0 - k) + k;

	return numerator / denominator;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughnessStrength)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);

	//float NdotV = dot(N, V);
	//float NdotL = dot(N, L);

	float GSubOne = GeometrySchlickGGX(NdotV, roughnessStrength);
	float GSubTwo = GeometrySchlickGGX(NdotL, roughnessStrength);

	return GSubOne * GSubTwo;
}

vec3 FresnelSchlick(float cos, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cos, 5.0);
}

//From Epic Games
vec3 FresnelSphericalGaussian(float cos, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(2.0, (-5.55473 * cos - 6.98316) * cos);
}

vec3 FresnelSchlickRoughness(float cos, vec3 F0, float roughnessStrength)
{
	return F0 + (max(vec3(1.0 - roughnessStrength), F0) - F0) * pow(clamp(1.0 - cos, 0.0, 1.0), 5.0);
}

void CalcDirectionalLight(inout vec3 result, vec3 albedoColor, float metallicStrength, float roughnessStrength, vec3 N, vec3 V, vec3 F0)
{
	vec3 L = normalize(-directionalLight.lightDirection.xyz);
	vec3 H = normalize(V + L);

	vec3 radiance = directionalLight.lightColor.xyz;

	float NDF = DistributionGGX(N, H, roughnessStrength);
	float G = GeometrySmith(N, V, L, roughnessStrength);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallicStrength;

	vec3 diffuse = kD * albedoColor / PI;

	//Cook-Torrance BRDF
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	vec3 specular = numerator / denominator;

	float NdotL = max(dot(N, L), 0.0001);
	float shadow = CalcShadows(NdotL);

	result = (diffuse + specular) * (1.0 - shadow) * radiance * NdotL;
}

void CalcPointLights(inout vec3 result, vec3 albedoColor, float metallicStrength, float roughnessStrength, vec3 N, vec3 V, vec3 F0)
{
	for(int i = 0; i < numPointLights; i++)
	{
		vec3 L = normalize(pointLights[i].lightPosition.xyz - IN.fragWorldPos);
		vec3 H = normalize(V + L);

		float distance = length(pointLights[i].lightPosition.xyz - IN.fragWorldPos);
		float attenuation = 1.0 / (distance * distance);
		//float attenuation = 1.0 / (pointLights[i].lightAttenData.x + pointLights[i].lightAttenData.y * distance + pointLights[i].lightAttenData.z * (distance * distance));
		vec3 radiance = pointLights[i].lightColor.xyz * attenuation;

		float NDF = DistributionGGX(N, H, roughnessStrength);
		float G = GeometrySmith(N, V, L, roughnessStrength);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallicStrength;

		//Cook-Torrance BRDF
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		float NdotL = max(dot(N, L), 0.0001);
		result += (kD * albedoColor / PI + specular) * radiance * NdotL;
	}
}

void CalcSpotLights(inout vec3 result, vec3 albedoColor, float metallicStrength, float roughnessStrength, vec3 N, vec3 V, vec3 F0)
{
	for(int i = 0; i < numSpotLights; i++)
	{
		vec3 L = normalize(spotLights[i].lightPosition.xyz - IN.fragWorldPos);
		vec3 H = normalize(V + L);

		float distance = length(spotLights[i].lightPosition.xyz - IN.fragWorldPos);
		//float attenuation = 1.0 / (spotLights[i].lightAttenData.x + spotLights[i].lightAttenData.y * distance + spotLights[i].lightAttenData.z * (distance * distance));
		float attenuation = 1.0 / (distance * distance);

		float theta = dot(L, normalize(-spotLights[i].lightDirection.xyz));
		float epsilon = spotLights[i].lightCutoffData.x - spotLights[i].lightCutoffData.y;
		float edgeFactor = clamp((theta - spotLights[i].lightCutoffData.y) / epsilon, 0.0, 1.0);
		
		vec3 radiance = spotLights[i].lightColor.xyz * attenuation * edgeFactor;

		float NDF = DistributionGGX(N, H, roughnessStrength);
		float G = GeometrySmith(N, V, L, roughnessStrength);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallicStrength;

		//Cook-Torrance BRDF
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		float NdotL = max(dot(N, L), 0.0001);
		result += (kD * albedoColor / PI + specular) * radiance * NdotL;
	}
}

void CalcAmbientLight(inout vec3 result, vec3 albedoColor, float metallicStrength, float roughnessStrength, vec3 N, vec3 V, vec3 R, vec3 F0)
{
	float NdotV = max(dot(N, V), 0.0);	
	vec3 F = FresnelSchlickRoughness(NdotV, F0, roughnessStrength);
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallicStrength;

	vec3 irradiance = texture(irradianceTex, N).rgb;
	vec3 diffuse = irradiance * albedoColor;	

	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilterTex, R, roughnessStrength * MAX_REFLECTION_LOD).rgb;
		
	vec2 envBRDF = texture(brdfLUTTex, vec2(NdotV, roughnessStrength)).rg;
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

	float aoTexStrength = hasOcclusionTex ? texture(occlusionTex, IN.texCoord).r : 1.0;

	//SSAO
	float aoStrength = 1.0;
	if(ssaoEnabled)
	{
		vec2 NDCSpaceFragPos = IN.fragClipSpacePos.xy / IN.fragClipSpacePos.w;
		vec2 texLoopUps = NDCSpaceFragPos * 0.5 + 0.5;
		aoStrength = texture(ssaoTex, texLoopUps).r;
	}

	//vec3 ambient = vec3(0.03) * albedoColor;
	vec3 ambient = (kD * diffuse + specular);
	ambient *= aoTexStrength;
	ambient *= aoStrength;

	result += ambient;
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
	}	
	normalColor = normalize(TBN * normalize(normalColor));

	//float metallicStrength = 1.0;
	//float roughnessStrength = 0.1;

	float metallicStrength = hasMetallicTex ? texture(metallicTex, IN.texCoord).r * u_Metallic : u_Metallic;
	float roughnessStrength = hasRoughnessTex ? texture(roughnessTex, IN.texCoord).r * u_Roughness : u_Roughness;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedoColor, metallicStrength);

	vec3 N = normalize(normalColor);
	vec3 V = normalize(cameraPos - IN.fragWorldPos);
	vec3 R = reflect(-V, N);

	vec3 result = vec3(0.0);
	CalcDirectionalLight(result, albedoColor, metallicStrength, roughnessStrength, N, V, F0);
	CalcPointLights(result, albedoColor, metallicStrength, roughnessStrength, N, V, F0);
	CalcSpotLights(result, albedoColor, metallicStrength, roughnessStrength, N, V, F0);
	CalcAmbientLight(result, albedoColor, metallicStrength, roughnessStrength, N, V, R, F0);

	result = vec3(1.0) - exp(-result * m_Exposure);
	result = pow(result, vec3(1.0 / m_GAMMA));

	if(hasEmissiveTex)
	{
		vec3 emissiveColor = texture(emissiveTex, IN.texCoord).rgb;
		result += emissiveColor * u_Emission;
	}

	//vec3 metallicColor = texture(metallicTex, IN.texCoord).rgb;
	//vec3 roughnessColor = texture(roughnessTex, IN.texCoord).rgb;

	fragColour = vec4(result, 1.0);
}