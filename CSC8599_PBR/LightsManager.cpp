#include "LightsManager.h"
#include "Renderer.h"
#include "LookAtCamera.h"
#include <nclgl/Shader.h>
#include <nclgl/UniformBuffer.h>
#include <nclgl/Light.h>
#include <nclgl/DirectionalLight.h>
#include <nclgl/SpotLight.h>
#include <nclgl/Texture.h>
#include <nclgl/common.h>

const int MAX_POINT_LIGHTS = 100;
const int MAX_SPOT_LIGHTS = 2;

LightsManager* LightsManager::m_LightsManager = nullptr;

LightsManager::LightsManager()
{
	m_PBRBillboardShader = std::shared_ptr<Shader>(new Shader("PBR/PBRBillboardVertex.glsl", "PBR/PBRBillboardFragment.glsl"));
	if (!m_PBRBillboardShader->LoadSuccess()) { m_IsInitialized = false; return; }

	m_LightIconTexture = std::shared_ptr<Texture>(new Texture(TEXTUREDIR + "Icons/Icon_Light.png"));
	if (m_LightIconTexture == 0) { m_IsInitialized = false; return; }	

	m_SpotLightIconTexture = std::shared_ptr<Texture>(new Texture(TEXTUREDIR + "Icons/Icon_SpotLight.png"));
	if (m_SpotLightIconTexture == 0) { m_IsInitialized = false; return; }

	//---------------------------------------------------------------------------------------------------------------------------------------
	//Point Lights
	m_PointLightsUBO = std::shared_ptr<UniformBuffer>(new UniformBuffer((MAX_POINT_LIGHTS * sizeof(PointLight)) + (sizeof(int) * 4), NULL, GL_DYNAMIC_DRAW, 1, 0));
	if (!m_PointLightsUBO->IsInitialized()) { m_IsInitialized = false; return; }

	//SpawnPointLight(Vector3(0, 1.0f, -2.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f));
	SpawnPointLight(Vector3(0, 1.0f, 1.5f), Vector4::RED);
	//SpawnPointLight(Vector3(-1.5f, 1.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	//SpawnPointLight(Vector3(1.5f, 1.0f, 0.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f));
	//---------------------------------------------------------------------------------------------------------------------------------------

	//---------------------------------------------------------------------------------------------------------------------------------------
	//Directional Lights
	m_DirectionalLightsUBO = std::shared_ptr<UniformBuffer>(new UniformBuffer(sizeof(DirectionalLightStruct), NULL, GL_DYNAMIC_DRAW, 2, 0));
	if (!m_DirectionalLightsUBO->IsInitialized()) { m_IsInitialized = false; return; }

	m_DirectionalLight = std::shared_ptr<DirectionalLight>(new DirectionalLight(Vector3(0.8574929257125442f, -0.5144957554275266f, 0.0f), Vector4::WHITE));
	if (m_DirectionalLight == nullptr) { m_IsInitialized = false; return; }

	m_DirectionalLightStruct = DirectionalLightStruct();
	m_DirectionalLightStruct.lightDirection = Vector4(m_DirectionalLight->GetLightDir(), 0.0f);
	m_DirectionalLightStruct.lightColor = m_DirectionalLight->GetColour();

	BindDirectionalLightUBOData();
	//---------------------------------------------------------------------------------------------------------------------------------------

	//---------------------------------------------------------------------------------------------------------------------------------------
	//Spot Lights
	m_SpotLightsUBO = std::shared_ptr<UniformBuffer>(new UniformBuffer((MAX_SPOT_LIGHTS * sizeof(SpotLightStruct)) + (sizeof(int) * 4), NULL, GL_DYNAMIC_DRAW, 3, 0));
	if (!m_SpotLightsUBO->IsInitialized()) { m_IsInitialized = false; return; }

	//SpawnSpotLight(Vector3::UP * 2.5f, Vector3::DOWN, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	//---------------------------------------------------------------------------------------------------------------------------------------

	ImGuiRenderer::Get()->RegisterItem(this);

	m_IsInitialized = true;

	m_LightsManager = this;
}

void LightsManager::SpawnPointLight()
{
	SpawnPointLight(Vector3(0, 1.0f, 0.0f), Vector4(1.0f, 1.0f, 1.0f, 1.0f));
}

void LightsManager::SpawnPointLight(const Vector3& lightPosition, const Vector4& lightColor)
{
	std::shared_ptr<Light> newPointLight = std::shared_ptr<Light>(new Light(lightPosition, lightColor, 2.0f));
	m_PointLights.insert(newPointLight);
	
	PointLight pLightStruct;
	pLightStruct.lightPosition = Vector4(newPointLight->GetPosition(), 1.0f);
	pLightStruct.lightColor = newPointLight->GetColour();
	pLightStruct.lightAttenuationData = Vector4(1.0f, 0.09f, 0.032f, 1.0f);
	
	m_PointLightsVec.push_back(pLightStruct);

	BindPointLightUBOData();
}

void LightsManager::SpawnSpotLight()
{
	SpawnSpotLight(Vector3::UP, Vector3::DOWN, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
}

void LightsManager::SpawnSpotLight(const Vector3& lightPosition, const Vector3& lightDir, const Vector4& lightColor)
{
	std::shared_ptr<SpotLight> newSpotLight = std::shared_ptr<SpotLight>(new SpotLight(lightPosition, lightDir, lightColor, 12.5f, 12.5f));
	m_SpotLights.insert(newSpotLight);

	SpotLightStruct sLightStruct;
	sLightStruct.lightPosition = Vector4(newSpotLight->GetPosition(), 1.0f);
	sLightStruct.lightDirection = Vector4(newSpotLight->GetLightDir(), 0.0f);
	sLightStruct.lightColor = lightColor;
	sLightStruct.lightAttenuationData = Vector4(1.0f, 0.09f, 0.032f, 0.0f);
	sLightStruct.lightCutOffData = Vector4(std::cos(DegToRad(newSpotLight->GetInnerCutOff())), std::cos(DegToRad(newSpotLight->GetOuterCutOff())), 0.0f, 0.0f);

	m_SpotLightsVec.push_back(sLightStruct);

	BindSpotLightUBOData();
}

void LightsManager::BindPointLightUBOData(int index)
{
	m_PointLightsUBO->Bind();

	if (index == -1)
	{
		int numLights[1] = { (int)m_PointLights.size() };
		m_PointLightsUBO->BindSubData(0, sizeof(int), numLights);
		m_PointLightsUBO->BindSubData(sizeof(int) * 4, sizeof(PointLight) * (int)m_PointLightsVec.size(), m_PointLightsVec.data());
	}
	else
	{
		/*
			16 + (32 * index)
			Here 16 being of start offset after NumPointLights
			Here 32 is size of PointLight struct
			Eg: Index At 5
				Offset for Index 5 = 16 + (32 * 5) = 16 + (160) = 176
		*/
		m_PointLightsUBO->BindSubData((sizeof(int) * 4) + (sizeof(PointLight) * index), sizeof(PointLight), &m_PointLightsVec[index]);
	}

	/*for (const auto& light : m_PointLights)
	{
		PointLight pLightStruct;
		pLightStruct.lightPosition = Vector4(light->GetPosition().x, light->GetPosition().y, light->GetPosition().z, 1.0f);
		pLightStruct.lightColor = light->GetColour();
		m_LightsUBO->BindSubData(sizeof(int) * 4, sizeof(PointLight), &pLightStruct);
	}*/
	m_PointLightsUBO->Unbind();
}

void LightsManager::BindDirectionalLightUBOData()
{
	m_DirectionalLightsUBO->Bind();
	m_DirectionalLightsUBO->BindSubData(0, sizeof(DirectionalLightStruct), &m_DirectionalLightStruct);
	m_DirectionalLightsUBO->Unbind();
}

void LightsManager::BindSpotLightUBOData(int index)
{
	m_SpotLightsUBO->Bind();

	if (index == -1)
	{
		int numLights[1] = { (int)m_SpotLights.size() };
		m_SpotLightsUBO->BindSubData(0, sizeof(int), numLights);
		m_SpotLightsUBO->BindSubData(sizeof(int) * 4, sizeof(SpotLightStruct) * (int)m_SpotLightsVec.size(), m_SpotLightsVec.data());
	}
	else
	{
		m_SpotLightsUBO->BindSubData((sizeof(int) * 4) + (sizeof(SpotLightStruct) * index), sizeof(SpotLightStruct), &m_SpotLightsVec[index]);
	}
	
	m_SpotLightsUBO->Unbind();
}

void LightsManager::Render()
{
	if ((int)m_PointLights.size() <= 0) return;

	m_PBRBillboardShader->Bind();
	m_PBRBillboardShader->SetTexture("mainTex", m_LightIconTexture->GetID(), 0);
	for (auto iter = m_PointLights.cbegin(); iter != m_PointLights.cend(); ++iter)
	{
		const auto& light = *iter;
		m_PBRBillboardShader->SetVector4("mainColor", light->GetColour());

		Vector3 look = Renderer::Get()->GetMainCamera()->GetPosition() - light->GetPosition();
		look.Normalise();

		//Point Billboards
		Vector3 right = Vector3::Cross(Renderer::Get()->GetMainCamera()->GetUp(), look);
		Vector3 up = Vector3::Cross(look, right);

		//Arbitrary Axis Billboards (Up Axis)
		/*Vector3 up = Vector3(0, 1, 0);
		Vector3 right = Vector3::Cross(up, look);
		right.Normalise();
		look = Vector3::Cross(right, up);*/

		//Axis Aligned Billboards (Y Axis)
		/*Vector3 up = Vector3::UP;
		Vector3 right = Vector3::Cross(up, look);*/

		Matrix4 billboardMat = Matrix4::Scale(0.2f) * Matrix4::CreateBillboardMatrix(right, up, look, light->GetPosition());
		m_PBRBillboardShader->SetMat4("billboardMatrix", billboardMat, true);

		Renderer::Get()->GetQuadMesh()->Draw();
	}

	m_PBRBillboardShader->SetTexture("mainTex", m_SpotLightIconTexture->GetID(), 0);
	for (auto iter = m_SpotLights.cbegin(); iter != m_SpotLights.cend(); ++iter)
	{
		const auto& light = *iter;
		m_PBRBillboardShader->SetVector4("mainColor", light->GetColour());

		Vector3 look = Renderer::Get()->GetMainCamera()->GetPosition() - light->GetPosition();
		look.Normalise();

		//Point Billboards
		Vector3 right = Vector3::Cross(Renderer::Get()->GetMainCamera()->GetUp(), look);
		Vector3 up = Vector3::Cross(look, right);

		//Arbitrary Axis Billboards (Up Axis)
		/*Vector3 up = Vector3(0, 1, 0);
		Vector3 right = Vector3::Cross(up, look);
		right.Normalise();
		look = Vector3::Cross(right, up);*/

		//Axis Aligned Billboards (Y Axis)
		/*Vector3 up = Vector3::UP;
		Vector3 right = Vector3::Cross(up, look);*/

		Matrix4 billboardMat = Matrix4::Scale(0.2f) * Matrix4::CreateBillboardMatrix(right, up, look, light->GetPosition());
		m_PBRBillboardShader->SetMat4("billboardMatrix", billboardMat, true);

		Renderer::Get()->GetQuadMesh()->Draw();
	}
	m_PBRBillboardShader->UnBind();
}

void LightsManager::OnImGuiRender()
{	
	if (ImGui::CollapsingHeader("Lights"))
	{
		ImGui::Indent();
		if (ImGui::CollapsingHeader("Directional Light") && m_DirectionalLight != nullptr)
		{
			ImGui::Indent();
			Vector3 m_LightDir = m_DirectionalLight->GetLightDir();
			if (ImGui::DragFloat3("Direction", (float*)&m_LightDir, 0.1f, -1.0f, 1.0f))
			{
				m_DirectionalLight->SetLightDir(m_LightDir);
				OnDirectionalLightPropertyChanged(m_LightDir, m_DirectionalLight->GetColour());
			}

			Vector4 m_LightColor = m_DirectionalLight->GetColour();
			if (ImGui::ColorEdit4("Color", (float*)&m_LightColor))
			{
				m_DirectionalLight->SetColour(m_LightColor);
				OnDirectionalLightPropertyChanged(m_DirectionalLight->GetLightDir(), m_LightColor);
			}
			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("Point Lights"))
		{
			if ((int)m_PointLights.size() > 0)
			{
				int i = 0;
				for (auto iter = m_PointLights.begin(); iter != m_PointLights.end(); ++iter)
				{
					ImGui::Indent();
					auto& light = *iter;
					const std::string lightHeaderStr = "Light - [" + std::to_string(i) + "]";
					if (ImGui::CollapsingHeader(lightHeaderStr.c_str()))
					{
						ImGui::Indent();
						Vector3 m_LightPos = light->GetPosition();
						if (ImGui::DragFloat3("Position", (float*)&m_LightPos))
						{
							light->SetPosition(m_LightPos);
							OnPointLightPropertyChanged(i, m_LightPos, light->GetColour());
						}

						Vector4 m_LightColor = light->GetColour();
						if (ImGui::ColorEdit4("Color", (float*)&m_LightColor))
						{
							light->SetColour(m_LightColor);
							OnPointLightPropertyChanged(i, light->GetPosition(), m_LightColor);
						}

						/*float m_LightRadius = light->GetRadius();
						if (ImGui::DragFloat("Radius", &m_LightRadius, 1.0f, 0.1f, 1000.0f)) light->SetRadius(m_LightRadius);*/

						ImGui::Unindent();
					}
					i++;

					ImGui::Unindent();
				}
			}
		
			ImGui::Separator();
			if ((int)m_PointLights.size() < MAX_POINT_LIGHTS)
				if (ImGui::Button("New Point Light")) 
					SpawnPointLight();
			ImGui::SameLine();
			if (ImGui::Button("Rebuild Light Data"))
			{
				BindDirectionalLightUBOData();
				BindPointLightUBOData();
			}
		}

		if (ImGui::CollapsingHeader("Spot Lights"))
		{
			if ((int)m_SpotLights.size() > 0)
			{
				int i = 0;
				for (auto iter = m_SpotLights.begin(); iter != m_SpotLights.end(); ++iter)
				{
					ImGui::Indent();
					auto& light = *iter;
					const std::string lightHeaderStr = "Spot Light - [" + std::to_string(i) + "]";
					if (ImGui::CollapsingHeader(lightHeaderStr.c_str()))
					{
						ImGui::Indent();
						Vector3 m_LightPos = light->GetPosition();
						if (ImGui::DragFloat3("Position", (float*)&m_LightPos, 0.25f))
						{
							light->SetPosition(m_LightPos);
							OnSpotLightPropertyChanged(i, m_LightPos, light->GetLightDir(), light->GetColour(), light->GetInnerCutOff(), light->GetOuterCutOff());
						}

						Vector3 m_LightDir = light->GetLightDir();
						if (ImGui::DragFloat3("Direction", (float*)&m_LightDir, 0.1f, -1.0f, 1.0f))
						{
							light->SetLightDir(m_LightDir);
							OnSpotLightPropertyChanged(i, light->GetPosition(), m_LightDir, light->GetColour(), light->GetInnerCutOff(), light->GetOuterCutOff());
						}

						Vector4 m_LightColor = light->GetColour();
						if (ImGui::ColorEdit4("Color", (float*)&m_LightColor))
						{
							light->SetColour(m_LightColor);
							OnSpotLightPropertyChanged(i, light->GetPosition(), light->GetLightDir(), m_LightColor, light->GetInnerCutOff(), light->GetOuterCutOff());
						}

						float m_LightInner = light->GetInnerCutOff();
						if (ImGui::DragFloat("Inner Radius", &m_LightInner, 0.1f, 1.0f, 180.0f))
						{
							light->SetInnerCutOff(m_LightInner);
							OnSpotLightPropertyChanged(i, light->GetPosition(), light->GetLightDir(), light->GetColour(), m_LightInner, light->GetOuterCutOff());
						}

						float m_LightOuter = light->GetOuterCutOff();
						if (ImGui::DragFloat("Outer Radius", &m_LightOuter, 0.1f, 1.0f, 100.0f))
						{
							light->SetOuterCutOff(m_LightOuter);
							OnSpotLightPropertyChanged(i, light->GetPosition(), light->GetLightDir(), light->GetColour(), light->GetInnerCutOff(), m_LightOuter);
						}
				
						ImGui::Unindent();
					}
					i++;
					ImGui::Unindent();
				}
			}

			ImGui::Separator();
			if((int)m_SpotLights.size() < MAX_SPOT_LIGHTS) 
				if (ImGui::Button("New Spot Light")) 
					SpawnSpotLight();
		}
		ImGui::Unindent();
	}
}

void LightsManager::OnPointLightPropertyChanged(int index, const Vector3& newLightPos, const Vector4& newLightColor)
{
	if (index == -1 || index >= (int)m_PointLights.size()) return;

	m_PointLightsVec[index].lightPosition = Vector4(newLightPos, 1.0f);
	m_PointLightsVec[index].lightColor = newLightColor;

	BindPointLightUBOData(index);
}

void LightsManager::OnDirectionalLightPropertyChanged(const Vector3& newLightDir, const Vector4& newLightColor)
{
	m_DirectionalLightStruct.lightDirection = Vector4(newLightDir, 0.0f);
	m_DirectionalLightStruct.lightColor = newLightColor;

	BindDirectionalLightUBOData();
}

void LightsManager::OnSpotLightPropertyChanged(int index, const Vector3& newLightPos, const Vector3& newLightDir, const Vector4& newLightColor, const float& innerCutOff, const float& outerCutOff)
{
	if (index == -1 || index >= (int)m_SpotLights.size()) return;

	m_SpotLightsVec[index].lightPosition = Vector4(newLightPos, 1.0f);
	m_SpotLightsVec[index].lightDirection = Vector4(newLightDir, 0.0f);
	m_SpotLightsVec[index].lightColor = newLightColor;
	m_SpotLightsVec[index].lightCutOffData = Vector4(std::cos(DegToRad(innerCutOff)), std::cos(DegToRad(outerCutOff)), 0.0f, 0.0f);

	BindSpotLightUBOData(index);
}