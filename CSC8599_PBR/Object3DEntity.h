#pragma once
#include <nclgl/Object3D.h>

struct MaterialData
{
	MaterialData() : m_BaseColor(Vector4::WHITE) {}
	MaterialData(const Vector4& baseColor) : m_BaseColor(baseColor) {}

	Vector4 m_BaseColor;
};

struct MaterialDataPBR : MaterialData
{
	MaterialDataPBR() : 
		MaterialData(Vector4::WHITE), 
		m_Metallic(1.0f),
		m_Roughness(1.0f),
		m_Emission(1.5f) {}

	MaterialDataPBR(const Vector4& baseColor, const float& metallic, const float& roughness, const float& emission) :
		MaterialData(baseColor),
		m_Metallic(metallic), 
		m_Roughness(roughness), 
		m_Emission(emission) {}

	float m_Metallic;
	float m_Roughness;
	float m_Emission;
};

struct MaterialDataDisney : MaterialData
{
	MaterialDataDisney() :
		MaterialData(Vector4::WHITE),
		m_Metallic(1.0f), m_Subsurface(0.0f),
		m_Roughness(1.0f),
		m_Specular(0.5f), m_SpecularTint(1.0f),
		m_Anisotropic(0.0f),
		m_Sheen(0.0f), m_SheenTint(1.0f),
		m_ClearCoat(0.0f), m_ClearCoatRoughness(1.0f),
		m_Emission(1.5f) {}

	MaterialDataDisney(const Vector4& baseColor,
						const float& metallic, const float& subSurface,
						const float& roughness,
						const float& specular, const float& specularTint,
						const float& anisotropic,
						const float& sheen, const float& sheenTint,
						const float& clearCoat, const float& clearCoatRoughness,
						const float& emission) :
						
						MaterialData(baseColor), 
						m_Metallic(metallic), m_Subsurface(subSurface),
						m_Roughness(roughness), 
						m_Specular(specular), m_SpecularTint(specularTint),
						m_Anisotropic(anisotropic),
						m_Sheen(sheen), m_SheenTint(sheenTint),
						m_ClearCoat(clearCoat), m_ClearCoatRoughness(clearCoatRoughness),
						m_Emission(emission) {}

	float m_Metallic;
	float m_Subsurface;
	float m_Roughness;
	float m_Specular;
	float m_SpecularTint;
	float m_Anisotropic;
	float m_Sheen;
	float m_SheenTint;
	float m_ClearCoat;
	float m_ClearCoatRoughness;
	float m_Emission;
};

struct MaterialDataBlinnPhong : MaterialData
{
	MaterialDataBlinnPhong() :
		MaterialData(Vector4::WHITE),
		m_Specular(5.0f),
		m_Shininess(128.0f),
		m_Emission(1.5f) {}

	MaterialDataBlinnPhong(const Vector4& baseColor, const float& specular, const float& specularShininess, const float& emission) :
		MaterialData(baseColor),
		m_Specular(specular),
		m_Shininess(specularShininess),
		m_Emission(emission) {}

	float m_Specular;
	float m_Shininess;
	float m_Emission;
};

struct MaterialDataOrenNayar : MaterialData
{
	MaterialDataOrenNayar() :
		MaterialData(Vector4::WHITE),
		m_SpecularEnabled(false),
		m_SpecularType(0),
		m_Roughness(1.0f),
		m_Emission(1.5f) {}

	MaterialDataOrenNayar(const Vector4& baseColor, const bool& enableSpecular, const int specularType, const float& roughness, const float& emission) :
		MaterialData(baseColor),
		m_SpecularEnabled(enableSpecular),
		m_SpecularType(specularType),
		m_Roughness(roughness),
		m_Emission(emission) {}

	bool m_SpecularEnabled;
	int m_SpecularType;		//0 - Gaussian Distribtion, 1 - Beckmann Distribution
	float m_Roughness;
	float m_Emission;
};

class SkyboxRenderer;
class PostProcessRenderer;
class Object3DRenderer;
class LookAtCamera;
class Object3DEntity : public Object3D
{
public:
	Object3DEntity(const std::string& objectName, const std::string& meshFileName, const std::string& meshMaterialName, const std::string& meshShaderVertexFile = "", const std::string& meshShaderFragmentFile = "", const float& lookAtDistance = 3.0f);
	~Object3DEntity() {}

	const int GetShaderMode() const { return m_ShaderMode; }
	void SetShaderMode(const int& shaderMode) { m_ShaderMode = shaderMode; }

	virtual void Render() override;

private:
	void RenderPBRMode();
	void RenderBlinnMode();
	void RenderDisneyMode();
	void RenderOrenNayarMode();

public:
	virtual void RenderShaderProperties();

private:
	std::shared_ptr<SkyboxRenderer> m_SkyboxRenderer;
	std::shared_ptr<PostProcessRenderer> m_PostProcessRenderer;
	std::shared_ptr<Object3DRenderer> m_Object3DRenderer;
	std::shared_ptr<LookAtCamera> m_MainCamera;

	int m_ShaderMode;		//0 - PBR, 1 - Blinn Phong, 2 - Disney

	MaterialDataPBR m_MaterialPBR;
	MaterialDataBlinnPhong m_MaterialBlinnPhong;
	MaterialDataDisney m_MaterialDisney;
	MaterialDataOrenNayar m_MaterialOrenNayar;	
};