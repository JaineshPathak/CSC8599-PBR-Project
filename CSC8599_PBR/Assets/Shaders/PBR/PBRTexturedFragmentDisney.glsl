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

//Disney Vars
uniform vec3 u_BaseColor = vec3(1.0);
uniform float u_Metallic = 0.01;
uniform float u_Subsurface = 0.0;
uniform float u_Roughness = 0.5;
uniform float u_Specular = 0.0;
uniform float u_SpecularTint = 0.0;
uniform float u_Anisotropic = 0.0;
uniform float u_Sheen = 1.0;
uniform float u_SheenTint = 0.5;
uniform float u_ClearCoat = 0.0;
uniform float u_ClearCoatRoughness = 1.0;
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

float sqr(float x) 
{ 
	return x * x;
}

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

float FresnelSchlick(float cos)
{
	float m = clamp(1.0 - cos, 0.0, 1.0);
	return (m * m) * (m * m) * m;
}

vec3 FresnelSchlickRoughness(float cos, vec3 F0, float roughnessStrength)
{
	return F0 + (max(vec3(1.0 - roughnessStrength), F0) - F0) * pow(clamp(1.0 - cos, 0.0, 1.0), 5.0);
}

//Distribution Term D for Clear Coat using Trowbridge-Reitz
float GTR1(float NdotH, float a)
{
	if(a >= 1.0) return 1.0 / PI;

	float a2 = a * a;
	float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;

	return (a2 - 1.0) / (PI * log(a2) * t);
}

float GTR2(float NdotH, float a)
{
	float a2 = a * a;
	float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;

	return a2 / (PI * t * t);
}

float GTR2_Aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
	return 1.0 / (PI * ax * ay * sqr( sqr(HdotX / ax) + sqr(HdotY / ay) + NdotH * NdotH ));
}

float SmithG_GGX(float NdotV, float alphaG)
{
	float a = alphaG * alphaG;
	float b = NdotV * NdotV;
	return 1.0 / (NdotV + sqrt(a + b - a * b));
}

float SmithG_GGX_Aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
	return 1.0 / (NdotV + sqrt( sqr(VdotX * ax) + sqr(VdotY * ay) + sqr(NdotV) ));
}

vec3 CalcTint(const in vec3 albedoColor)
{
	//float Luminance = dot(vec3(0.3, 0.6, 0.1), albedoColor);
	float Luminance = 0.3 * albedoColor.r + 0.6 * albedoColor.g + 0.1 * albedoColor.b;
	vec3 Tint = (Luminance > 0) ? albedoColor * (1.0 / Luminance) : vec3(1.0);
	
	return Tint;
}

//X - Tangent, Y - BiTangent or BiNormal
//Diffuse
vec3 CalcDiffuse(const in vec3 albedoColor, const in float roughnessStrength, const in float NdotL, const in float NdotV, const in float LdotH)
{
	float FLambert = 1.0 / PI;
	float FL = FresnelSchlick(NdotL);
	float FV = FresnelSchlick(NdotV);

	float Rr = 2.0 * roughnessStrength * LdotH * LdotH;
	float FRetro = FLambert * Rr * (FL + FV + FL * FV * (Rr - 1.0));

	float FDiffuse = FLambert * (1.0 - 0.5 * FL) * (1.0 - 0.5 * FV) + FRetro;
	return FDiffuse * albedoColor;
}

//Diffuse Flatten
vec3 CalcDiffuseSubsurface(const in vec3 albedoColor, const in float roughnessStrength, const in float NdotL, const in float NdotV, const in float LdotH)
{
	float FLambert = 1.0 / PI;
	float FL = FresnelSchlick(NdotL);
	float FV = FresnelSchlick(NdotV);

	float Fss90 = LdotH * LdotH * roughnessStrength;
	float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
	float FSubsurface = 1.25 * (Fss * (1.0 / (NdotL + NdotV) - 0.5) + 0.5);

	return FLambert * FSubsurface * albedoColor;
}

vec3 CalcSpecularIsotropic(const in vec3 albedoColor, const in float metallicStrength, const in float roughnessStrength, float NdotL, float NdotV, float NdotH, float LdotH)
{
	vec3 cTint = CalcTint(albedoColor);
	vec3 cSpec0 = mix(u_Specular * 0.08 * mix(vec3(1.0), cTint, u_SpecularTint), albedoColor, metallicStrength);

	float a = max(0.001, sqr(roughnessStrength));
	float Ds = GTR2(NdotH, a);
	float FH = FresnelSchlick(LdotH);

	vec3 Fs = mix(cSpec0, vec3(1.0), FH);
	float Gs;
	Gs  = SmithG_GGX(NdotL, a);
	Gs *= SmithG_GGX(NdotV, a);

	return Gs * Fs * Ds;
}

vec3 CalcSpecularAnisotropic(const in vec3 albedoColor, const in float metallicStrength, const in float roughnessStrength, 
								 float NdotL, float NdotV, float NdotH, float LdotH,
								 const in vec3 L, const in vec3 V, const in vec3 H,
								 const in vec3 X, const in vec3 Y)
{
	vec3 cTint = CalcTint(albedoColor);
	vec3 cSpec0 = mix(u_Specular * 0.08 * mix(vec3(1.0), cTint, u_SpecularTint), albedoColor, metallicStrength);

	float aspect = sqrt(1.0 - u_Anisotropic * 0.9);
	float ax = max(0.001, sqr(roughnessStrength) / aspect);
	float ay = max(0.001, sqr(roughnessStrength) * aspect);

	float Ds = GTR2_Aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
	float FH = FresnelSchlick(LdotH);
	vec3 Fs = mix(cSpec0, vec3(1.0), FH);

	float Gs;
	Gs =  SmithG_GGX_Aniso(NdotL, dot(L, X), dot(L, Y), ax, ay);
	Gs *= SmithG_GGX_Aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);

	return Gs * Fs * Ds;
}

vec3 CalcSheen(const in vec3 albedoColor, float LdotH)
{
	if(u_Sheen <= 0.0)
		return vec3(0.0);

	float FresLdotH = FresnelSchlick(LdotH);
	vec3 Tint = CalcTint(albedoColor);
	vec3 Sheen = u_Sheen * mix(vec3(1.0), albedoColor, u_SheenTint) * FresLdotH;
}

float CalcClearCoat(float NdotL, float NdotV, float NdotH, float LdotH)
{
	float rough = mix(0.1, 0.001, u_ClearCoatRoughness);
	float Dr = GTR1(abs(NdotH), rough);
	float FH = FresnelSchlick(LdotH);
	float Fr = mix(0.04, 1.0, FH);
	float Gr = SmithG_GGX(NdotL, 0.25) * SmithG_GGX(NdotV, 0.25);

	return u_ClearCoat * Fr * Gr * Dr;
}

//w0 = L
//wi = V
//wm = H
//ThetaD = H dot V -> max(HdotV, 0.0) used in Fresnel Schlick
void CalcDirectionalLight(inout vec3 result, vec3 albedoColor, float metallicStrength, float roughnessStrength, vec3 N, vec3 V, vec3 F0)
{
	vec3 X = IN.tangent;
	vec3 Y = IN.bitangent;
	vec3 radiance = directionalLight.lightColor.xyz;

	vec3 L = normalize(-directionalLight.lightDirection.xyz);
	vec3 H = normalize(V + L);
	
	float NdotV = abs(dot(N, V)) + 1e-5;
	float NdotL = clamp(dot(N, L), 0.001, 1.0);
	float NdotH = clamp(dot(N, H), 0.001, 1.0);
	float LdotH = clamp(dot(L, H), 0.001, 1.0);
	float HdotV = clamp(dot(H, V), 0.001, 1.0);

	vec3 FDiffuse = CalcDiffuse(albedoColor, roughnessStrength, NdotL, NdotV, LdotH);
	vec3 FDiffuseSubsurface = CalcDiffuseSubsurface(albedoColor, roughnessStrength, NdotL, NdotV, LdotH);

	vec3 FSpecular = CalcSpecularAnisotropic(albedoColor, metallicStrength, roughnessStrength, NdotL, NdotV, NdotH, LdotH, L, V, H, X, Y);
	//vec3 FSpecular = CalcSpecularIsotropic(albedoColor, metallicStrength, roughnessStrength, NdotL, NdotV, NdotH, LdotH);

	vec3 FSheen = mix(vec3(1.0), albedoColor, u_SheenTint) * u_Sheen * FresnelSchlick(LdotH);	
	float FClearcoat = CalcClearCoat(NdotL, NdotV, NdotH, LdotH);

	vec3 FDiffuseFinal = mix(FDiffuse, FDiffuseSubsurface, u_Subsurface);
	vec3 FSpecularFinal = FSpecular + FClearcoat;	

	//result = (FDiffuseFinal + FSheen + FClearcoat) * radiance * NdotL;
	//result = FSpecular * radiance * NdotL;

	float NoL = max(dot(N, L), 0.0001);
	float shadow = CalcShadows(NoL);

	result = ((FDiffuseFinal + FSheen) * (1.0 - metallicStrength) + FSpecularFinal) * radiance * NdotL;
	result *= (1.0 - shadow);
	//result *= radiance * NdotL;		//So that it doesn't apply on the whole model and only where the light ray falls on model surface
}

void CalcPointLights(inout vec3 result, vec3 albedoColor, float metallicStrength, float roughnessStrength, vec3 N, vec3 V, vec3 F0)
{
	vec3 X = IN.tangent;
	vec3 Y = IN.bitangent;

	for(int i = 0; i < numPointLights; i++)
	{
		float distance = length(pointLights[i].lightPosition.xyz - IN.fragWorldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = pointLights[i].lightColor.xyz * attenuation;

		vec3 L = normalize(pointLights[i].lightPosition.xyz - IN.fragWorldPos);
		vec3 H = normalize(V + L);
		
		float NdotV = abs(dot(N, V)) + 1e-5;
		float NdotL = clamp(dot(N, L), 0.001, 1.0);
		float NdotH = clamp(dot(N, H), 0.001, 1.0);
		float LdotH = clamp(dot(L, H), 0.001, 1.0);
		float HdotV = clamp(dot(H, V), 0.001, 1.0);		

		vec3 FDiffuse = CalcDiffuse(albedoColor, roughnessStrength, NdotL, NdotV, LdotH);
		vec3 FDiffuseSubsurface = CalcDiffuseSubsurface(albedoColor, roughnessStrength, NdotL, NdotV, LdotH);

		vec3 FSpecular = CalcSpecularAnisotropic(albedoColor, metallicStrength, roughnessStrength, NdotL, NdotV, NdotH, LdotH, L, V, H, X, Y);
		//vec3 FSpecular = CalcSpecularIsotropic(albedoColor, metallicStrength, roughnessStrength, NdotL, NdotV, NdotH, LdotH);

		vec3 FSheen = mix(vec3(1.0), albedoColor, u_SheenTint) * u_Sheen * FresnelSchlick(LdotH);	
		float FClearcoat = CalcClearCoat(NdotL, NdotV, NdotH, LdotH);

		vec3 FDiffuseFinal = mix(FDiffuse, FDiffuseSubsurface, u_Subsurface);
		vec3 FSpecularFinal = FSpecular + FClearcoat;

		result += ((FDiffuseFinal + FSheen) * (1.0 - metallicStrength) + FSpecularFinal) * radiance * NdotL;
	}
}

void CalcSpotLights(inout vec3 result, vec3 albedoColor, float metallicStrength, float roughnessStrength, vec3 N, vec3 V, vec3 F0)
{
	vec3 X = IN.tangent;
	vec3 Y = IN.bitangent;

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

		float NdotV = abs(dot(N, V)) + 1e-5;
		float NdotL = clamp(dot(N, L), 0.001, 1.0);
		float NdotH = clamp(dot(N, H), 0.001, 1.0);
		float LdotH = clamp(dot(L, H), 0.001, 1.0);
		float HdotV = clamp(dot(H, V), 0.001, 1.0);		

		vec3 FDiffuse = CalcDiffuse(albedoColor, roughnessStrength, NdotL, NdotV, LdotH);
		vec3 FDiffuseSubsurface = CalcDiffuseSubsurface(albedoColor, roughnessStrength, NdotL, NdotV, LdotH);

		vec3 FSpecular = CalcSpecularAnisotropic(albedoColor, metallicStrength, roughnessStrength, NdotL, NdotV, NdotH, LdotH, L, V, H, X, Y);
		//vec3 FSpecular = CalcSpecularIsotropic(albedoColor, metallicStrength, roughnessStrength, NdotL, NdotV, NdotH, LdotH);

		vec3 FSheen = mix(vec3(1.0), albedoColor, u_SheenTint) * u_Sheen * FresnelSchlick(LdotH);	
		float FClearcoat = CalcClearCoat(NdotL, NdotV, NdotH, LdotH);

		vec3 FDiffuseFinal = mix(FDiffuse, FDiffuseSubsurface, u_Subsurface);
		vec3 FSpecularFinal = FSpecular + FClearcoat;

		result += ((FDiffuseFinal + FSheen) * (1.0 - metallicStrength) + FSpecularFinal) * radiance * NdotL;		
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

	fragColour = vec4(result, 1.0);
}