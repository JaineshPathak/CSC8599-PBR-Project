#version 420 core

//Textures
uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D roughnessTex;
uniform sampler2D emissiveTex;
uniform sampler2D occlusionTex;

uniform samplerCube irradianceTex;
uniform sampler2D ssaoTex;
uniform sampler2D shadowTex;

//Flags
uniform bool ssaoEnabled;
uniform bool hasAlbedoTex = false;
uniform bool hasNormalTex = false;
uniform bool hasRoughnessTex = false;
uniform bool hasEmissiveTex = false;
uniform bool hasOcclusionTex = false;

uniform vec3 u_BaseColor = vec3(1.0);
uniform bool u_SpecularEnabled = false;
uniform int u_SpecularType = 0;				//0 - Gaussian, 1 - Beckmann
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

float CalcGaussianDistribution(const in float roughnessStrength, const in float NdotH)
{
	float alpha = acos(NdotH);
	float m = roughnessStrength;

	float exponent = alpha / m;
	float exponent2 = -(exponent * exponent);

	return exp(exponent2);
}

float CalcBeckmannDistribution(const in float roughnessStrength, const in float NdotH)
{	
	float T = NdotH * NdotH;
	float M = roughnessStrength * roughnessStrength;

	float numerator = exp((T - 1.0) / (T * M)); 
	float denominator = PI * M * T * T;

	return numerator / denominator;
}

float CalcGGX(const in float roughnessStrength, const in float NdotH)
{
	float Alpha = roughnessStrength * roughnessStrength;
	float AlphaMinus = Alpha - 1.0;
	float NdotHSq = NdotH * NdotH;

	float numerator = Alpha;
	float denominator = PI * pow(NdotHSq * AlphaMinus + 1.0, 2.0);

	return numerator / denominator;
}

//cos i = NdotL
//cos r = NdotV
void CalcDirectionalLight(inout vec3 result, in vec3 albedoColor, in float roughnessStrength, in vec3 N, in vec3 V)
{
	vec3 L = normalize(-directionalLight.lightDirection.xyz);
	vec3 H = normalize(V + L);

	vec3 radiance = directionalLight.lightColor.xyz;

	float NdotL = clamp(dot(N, L), 0.001, 1.0);
	float NdotV = clamp(dot(N, V), 0.001, 1.0);
	float NdotH = clamp(dot(N, H), 0.001, 1.0);
	float thetaNL = acos(NdotL);
	float thetaNV = acos(NdotV);

	float alpha = max(thetaNL, thetaNV);
	float beta = min(thetaNL, thetaNV);
	float gamma = max(cos(thetaNV - thetaNL), 0.0);

	float sinAlpha = sin(alpha);
	float r = clamp(roughnessStrength, 0.01, 1.0);
	float roughness2 = r * r;

	/*float C1 = 1.0 - 0.5 * roughness2 / (roughness2 + 0.33);
	
	float C2 = 0.45 * roughness2 / (roughness2 + 0.09);
	if(gamma >= 0.0)
		C2 *= sinAlpha;
	else
		C2 *= sinAlpha - pow((2.0 * beta / PI), 3.0);
	
	float C3 = 0.125 * roughness2 / (roughness2 + 0.09) * pow( (4.0 * alpha * beta) / (PI * PI), 2.0);

	vec3 albedoColor2 = (albedoColor * albedoColor);

	vec3 L1 = albedoColor / PI * (C1 + C2 * gamma * tan(beta) + C3 * (1.0 - abs(gamma)) * tan(alpha + beta / 2.0));
	vec3 L2 = 0.17 * albedoColor2 / PI * roughness2 / (roughness2 + 0.13) * (1.0 - gamma * pow(2.0 * beta / PI, 2.0));
	result = (L1 + L2) * radiance * NdotL;*/

	float C1 = 1.0 - 0.5 * roughness2 / (roughness2 + 0.57);
	float C2 = 0.45 * roughness2 / (roughness2 + 0.09);
	float C3 = sin(alpha) * tan(beta);

	float FDiffuse = (1.0 / PI) * (C1 + (C2 * gamma * C3));
	float FSpecular = 0.0f;
	if(u_SpecularEnabled)
	{
		switch(u_SpecularType)
		{
			case 0: { FSpecular = CalcGaussianDistribution(r, NdotH); break; }
			case 1: { FSpecular = CalcBeckmannDistribution(r, NdotH); break; }
			case 2: { FSpecular = CalcGGX(r, NdotH); break; }
			default: { FSpecular = 0.0f; break; }
		}
	}

	float NoL = max(dot(N, L), 0.0001);
	float shadow = CalcShadows(NoL);
	result = (FDiffuse + FSpecular) * (1.0 - shadow) * albedoColor * radiance * NoL;	
}

void CalcPointsLights(inout vec3 result, in vec3 albedoColor, in float roughnessStrength, in vec3 N, in vec3 V)
{
	for(int i = 0; i < numPointLights; i++)
	{
		vec3 L = normalize(pointLights[i].lightPosition.xyz - IN.fragWorldPos);
		vec3 H = normalize(V + L);

		float distance = length(pointLights[i].lightPosition.xyz - IN.fragWorldPos);
		float attenuation = 1.0 / (distance * distance);

		vec3 radiance = pointLights[i].lightColor.xyz * attenuation;

		float NdotL = clamp(dot(N, L), 0.001, 1.0);
		float NdotV = clamp(dot(N, V), 0.001, 1.0);
		float NdotH = clamp(dot(N, H), 0.001, 1.0);
		float thetaNL = acos(NdotL);
		float thetaNV = acos(NdotV);

		float alpha = max(thetaNL, thetaNV);
		float beta = min(thetaNL, thetaNV);
		float gamma = max(cos(thetaNV - thetaNL), 0.0);

		float sinAlpha = sin(alpha);
		float r = clamp(roughnessStrength, 0.01, 1.0);
		float roughness2 = r * r;

		float C1 = 1.0 - 0.5 * roughness2 / (roughness2 + 0.57);
		float C2 = 0.45 * roughness2 / (roughness2 + 0.09);
		float C3 = sin(alpha) * tan(beta);

		float FDiffuse = (1.0 / PI) * (C1 + (C2 * gamma * C3));
		float FSpecular = 0.0f;
		if(u_SpecularEnabled)
		{
			switch(u_SpecularType)
			{
				case 0: { FSpecular = CalcGaussianDistribution(r, NdotH); break; }
				case 1: { FSpecular = CalcBeckmannDistribution(r, NdotH); break; }
				case 2: { FSpecular = CalcGGX(r, NdotH); break; }
				default: { FSpecular = 0.0f; break; }
			}
		}
	
		result += (FDiffuse + FSpecular) * albedoColor * radiance * NdotL;
	}
}

void CalcSpotLights(inout vec3 result, in vec3 albedoColor, in float roughnessStrength, in vec3 N, in vec3 V)
{
	for(int i = 0; i < numSpotLights; i++)
	{
		vec3 L = normalize(spotLights[i].lightPosition.xyz - IN.fragWorldPos);
		vec3 H = normalize(V + L);

		float distance = length(spotLights[i].lightPosition.xyz - IN.fragWorldPos);
		float attenuation = 1.0 / (distance * distance);

		float theta = dot(L, normalize(-spotLights[i].lightDirection.xyz));
		float epsilon = spotLights[i].lightCutoffData.x - spotLights[i].lightCutoffData.y;
		float edgeFactor = clamp((theta - spotLights[i].lightCutoffData.y) / epsilon, 0.0, 1.0);
		
		vec3 radiance = spotLights[i].lightColor.xyz * attenuation * edgeFactor;

		float NdotL = clamp(dot(N, L), 0.001, 1.0);
		float NdotV = clamp(dot(N, V), 0.001, 1.0);
		float NdotH = clamp(dot(N, H), 0.001, 1.0);
		float thetaNL = acos(NdotL);
		float thetaNV = acos(NdotV);

		float alpha = max(thetaNL, thetaNV);
		float beta = min(thetaNL, thetaNV);
		float gamma = max(cos(thetaNV - thetaNL), 0.0);

		float sinAlpha = sin(alpha);
		float r = clamp(roughnessStrength, 0.01, 1.0);
		float roughness2 = r * r;

		float C1 = 1.0 - 0.5 * roughness2 / (roughness2 + 0.57);
		float C2 = 0.45 * roughness2 / (roughness2 + 0.09);
		float C3 = sin(alpha) * tan(beta);

		float FDiffuse = (1.0 / PI) * (C1 + (C2 * gamma * C3));
		float FSpecular = 0.0f;
		if(u_SpecularEnabled)
		{
			switch(u_SpecularType)
			{
				case 0: { FSpecular = CalcGaussianDistribution(r, NdotH); break; }
				case 1: { FSpecular = CalcBeckmannDistribution(r, NdotH); break; }
				case 2: { FSpecular = CalcGGX(r, NdotH); break; }
				default: { FSpecular = 0.0f; break; }
			}
		}
	
		result += (FDiffuse + FSpecular) * albedoColor * radiance * NdotL;
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

	float roughnessStrength = hasRoughnessTex ? texture(roughnessTex, IN.texCoord).r * u_Roughness : u_Roughness;

	vec3 N = normalize(normalColor);
	vec3 V = normalize(cameraPos - IN.fragWorldPos);

	vec3 result = vec3(0.0);
	CalcDirectionalLight(result, albedoColor, roughnessStrength, N, V);
	CalcPointsLights(result, albedoColor, roughnessStrength, N, V);
	CalcSpotLights(result, albedoColor, roughnessStrength, N, V);
	CalcAmbientLight(result, albedoColor, normalColor);

	//Gamma	
	result = vec3(1.0) - exp(-result * m_Exposure);
	result = pow(result, vec3(1.0 / m_GAMMA));

	if(hasEmissiveTex)
	{
		vec3 emissiveColor = texture(emissiveTex, IN.texCoord).rgb;
		result += emissiveColor * u_Emission;
	}

	fragColour = vec4(result, 1.0);
}