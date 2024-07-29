#pragma once
#include "IImguiItem.h"
#include "ImGuiRenderer.h"
#include <memory>
#include <unordered_set>
#include <vector>

class UniformBuffer;
class Shader;
class Light;
class DirectionalLight;
class SpotLight;
class Texture;

struct PointLight
{
	Vector4 lightPosition;
	Vector4 lightColor;
	Vector4 lightAttenuationData;	//x = Kc (Constant), y = Kl (Linear), z = Kq (Quadratic), w = 1.0
};

struct DirectionalLightStruct
{
	Vector4 lightDirection;
	Vector4 lightColor;
};

struct SpotLightStruct
{
	Vector4 lightPosition;
	Vector4 lightDirection;
	Vector4 lightColor;
	Vector4 lightAttenuationData;	//x = Kc (Constant), y = Kl (Linear), z = Kq (Quadratic), w = 1.0
	Vector4 lightCutOffData;
};

class LightsManager : public IImguiItem
{
public:
	static LightsManager* Get() { return m_LightsManager; }

	LightsManager();
	~LightsManager() {};

	const bool IsInitialized() const { return m_IsInitialized; }

	std::shared_ptr<DirectionalLight> GetDirectionalLight() { return m_DirectionalLight; }

	void SpawnPointLight();
	void SpawnPointLight(const Vector3& lightPosition, const Vector4& lightColor);

	void SpawnSpotLight();
	void SpawnSpotLight(const Vector3& lightPosition, const Vector3& lightDir, const Vector4& lightColor);

	void BindPointLightUBOData(int index = -1);
	void BindDirectionalLightUBOData();
	void BindSpotLightUBOData(int index = -1);
	void Render();

	virtual void OnImGuiRender() override;

protected:
	void OnPointLightPropertyChanged(int index, const Vector3& newLightPos, const Vector4& newLightColor);
	void OnDirectionalLightPropertyChanged(const Vector3& newLightDir, const Vector4& newLightColor);
	void OnSpotLightPropertyChanged(int index, const Vector3& newLightPos, const Vector3& newLightDir, const Vector4& newLightColor, const float& innerCutOff, const float& outerCutOff);

protected:
	static LightsManager* m_LightsManager;

	bool m_IsInitialized;
	std::shared_ptr<Texture> m_LightIconTexture;
	std::shared_ptr<Texture> m_SpotLightIconTexture;
	
	std::shared_ptr<UniformBuffer> m_PointLightsUBO;
	std::shared_ptr<UniformBuffer> m_DirectionalLightsUBO;
	std::shared_ptr<UniformBuffer> m_SpotLightsUBO;
	
	std::shared_ptr<Shader> m_PBRBillboardShader;
	
	std::unordered_set<std::shared_ptr<Light>> m_PointLights;
	std::vector<PointLight> m_PointLightsVec;

	std::shared_ptr<DirectionalLight> m_DirectionalLight;
	DirectionalLightStruct m_DirectionalLightStruct;

	std::unordered_set<std::shared_ptr<SpotLight>> m_SpotLights;
	std::vector<SpotLightStruct> m_SpotLightsVec;
};