#include "Object3DEntity.h"

#include "Renderer.h"
#include "SkyboxRenderer.h"
#include "PostProcessRenderer.h"
#include "Object3DRenderer.h"
#include "LookAtCamera.h"

#include <nclgl/TextureEnvCubeMap.h>

Object3DEntity::Object3DEntity(const std::string& objectName, const std::string& meshFileName, const std::string& meshMaterialName, const std::string& meshShaderVertexFile, const std::string& meshShaderFragmentFile, const float& lookAtDistance) :	
	m_ShaderMode(0),	
	Object3D(objectName, meshFileName, meshMaterialName, meshShaderVertexFile, meshShaderFragmentFile, lookAtDistance)
{
	m_SkyboxRenderer = Renderer::Get()->GetSkyboxRenderer();
	m_PostProcessRenderer = Renderer::Get()->GetPostProcessRenderer();
	m_Object3DRenderer = Renderer::Get()->GetObject3DRenderer();
	m_MainCamera = Renderer::Get()->GetMainCamera();
}

void Object3DEntity::Render()
{
	if(m_SkyboxRenderer == nullptr) m_SkyboxRenderer = Renderer::Get()->GetSkyboxRenderer();
	if(m_PostProcessRenderer == nullptr) m_PostProcessRenderer = Renderer::Get()->GetPostProcessRenderer();
	if (m_Object3DRenderer == nullptr) m_Object3DRenderer = Renderer::Get()->GetObject3DRenderer();

	switch (m_ShaderMode)
	{
		case 0: RenderPBRMode(); break;
		case 1: RenderBlinnMode(); break;
		case 2: RenderDisneyMode(); break;
		case 3: RenderOrenNayarMode(); break;
		default: RenderPBRMode(); break;
	}
}

void Object3DEntity::RenderPBRMode()
{
	m_ShaderObject->Bind();

	m_ShaderObject->SetTextureCubeMap("irradianceTex", m_SkyboxRenderer->GetIrradianceTexture()->GetID(), 6);
	m_ShaderObject->SetTextureCubeMap("prefilterTex", m_SkyboxRenderer->GetPreFilterTexture()->GetID(), 7);
	m_ShaderObject->SetTexture("brdfLUTTex", m_SkyboxRenderer->GetBRDFLUTTexture()->GetID(), 8);

	m_ShaderObject->SetTexture("ssaoTex", m_PostProcessRenderer->GetSSAOProcessedTexture(), 9);
	m_ShaderObject->SetInt("ssaoEnabled", m_PostProcessRenderer->IsSSAOEnabled() && m_PostProcessRenderer->IsEnabled());

	m_ShaderObject->SetTexture("shadowTex", m_Object3DRenderer->GetShadowDepthTexture(), 10);

	m_ShaderObject->SetVector3("cameraPos", m_MainCamera->GetPosition());
	m_ShaderObject->SetMat4("lightSpaceMatrix", m_Object3DRenderer->GetLightSpaceMatrix());
	m_ShaderObject->SetMat4("modelMatrix", m_ModelMatrix);

	for (int i = 0; i < m_MeshObject->GetSubMeshCount(); i++)
	{
		m_ShaderObject->SetBool("hasAlbedoTex", m_TexDiffuseSet[i] != -1);
		if (m_TexDiffuseSet[i] != -1) m_ShaderObject->SetTexture("albedoTex", m_TexDiffuseSet[i], 0);

		m_ShaderObject->SetBool("hasNormalTex", m_TexNormalSet[i] != -1);
		if (m_TexNormalSet[i] != -1) m_ShaderObject->SetTexture("normalTex", m_TexNormalSet[i], 1);

		m_ShaderObject->SetBool("hasMetallicTex", m_TexMetallicSet[i] != -1);
		if (m_TexMetallicSet[i] != -1) m_ShaderObject->SetTexture("metallicTex", m_TexMetallicSet[i], 2);

		m_ShaderObject->SetBool("hasRoughnessTex", m_TexRoughnessSet[i] != -1);
		if (m_TexRoughnessSet[i] != -1) m_ShaderObject->SetTexture("roughnessTex", m_TexRoughnessSet[i], 3);

		m_ShaderObject->SetBool("hasEmissiveTex", m_TexEmissionSet[i] != -1);
		if (m_TexEmissionSet[i] != -1) m_ShaderObject->SetTexture("emissiveTex", m_TexEmissionSet[i], 4);

		m_ShaderObject->SetBool("hasOcclusionTex", m_TexOcclusionSet[i] != -1);
		if (m_TexOcclusionSet[i] != -1) m_ShaderObject->SetTexture("occlusionTex", m_TexOcclusionSet[i], 5);

		Vector3 baseColor = Vector3(m_MaterialPBR.m_BaseColor.x, m_MaterialPBR.m_BaseColor.y, m_MaterialPBR.m_BaseColor.z);
		m_ShaderObject->SetVector3("u_BaseColor", baseColor);
		m_ShaderObject->SetFloat("u_Metallic", m_MaterialPBR.m_Metallic);
		m_ShaderObject->SetFloat("u_Roughness", m_MaterialPBR.m_Roughness);
		m_ShaderObject->SetFloat("u_Emission", m_MaterialPBR.m_Emission);

		m_MeshObject->DrawSubMesh(i);
	}

	m_ShaderObject->UnBind();
}

void Object3DEntity::RenderBlinnMode()
{
	m_ShaderObject->Bind();

	m_ShaderObject->SetTextureCubeMap("cubeTex", m_SkyboxRenderer->GetEnvCubeMapTexture()->GetID(), 4);
	m_ShaderObject->SetTextureCubeMap("irradianceTex", m_SkyboxRenderer->GetIrradianceTexture()->GetID(), 5);

	m_ShaderObject->SetInt("ssaoEnabled", m_PostProcessRenderer->IsSSAOEnabled() && m_PostProcessRenderer->IsEnabled());
	m_ShaderObject->SetTexture("ssaoTex", m_PostProcessRenderer->GetSSAOProcessedTexture(), 6);

	m_ShaderObject->SetTexture("shadowTex", m_Object3DRenderer->GetShadowDepthTexture(), 7);

	m_ShaderObject->SetVector3("cameraPos", m_MainCamera->GetPosition());

	m_ShaderObject->SetMat4("lightSpaceMatrix", m_Object3DRenderer->GetLightSpaceMatrix());
	m_ShaderObject->SetMat4("modelMatrix", m_ModelMatrix);

	for (int i = 0; i < m_MeshObject->GetSubMeshCount(); i++)
	{
		m_ShaderObject->SetBool("hasAlbedoTex", m_TexDiffuseSet[i] != -1);
		if (m_TexDiffuseSet[i] != -1) m_ShaderObject->SetTexture("albedoTex", m_TexDiffuseSet[i], 0);

		m_ShaderObject->SetBool("hasNormalTex", m_TexNormalSet[i] != -1);
		if (m_TexNormalSet[i] != -1) m_ShaderObject->SetTexture("normalTex", m_TexNormalSet[i], 1);

		m_ShaderObject->SetBool("hasEmissiveTex", m_TexEmissionSet[i] != -1);
		if (m_TexEmissionSet[i] != -1) m_ShaderObject->SetTexture("emissiveTex", m_TexEmissionSet[i], 2);

		m_ShaderObject->SetBool("hasOcclusionTex", m_TexOcclusionSet[i] != -1);
		if (m_TexOcclusionSet[i] != -1) m_ShaderObject->SetTexture("occlusionTex", m_TexOcclusionSet[i], 3);

		Vector3 baseColor = Vector3(m_MaterialBlinnPhong.m_BaseColor.x, m_MaterialBlinnPhong.m_BaseColor.y, m_MaterialBlinnPhong.m_BaseColor.z);
		m_ShaderObject->SetVector3("u_BaseColor", baseColor);
		m_ShaderObject->SetFloat("u_Specular", m_MaterialBlinnPhong.m_Specular);
		m_ShaderObject->SetFloat("u_Shininess", m_MaterialBlinnPhong.m_Shininess);
		m_ShaderObject->SetFloat("u_Emission", m_MaterialBlinnPhong.m_Emission);

		m_MeshObject->DrawSubMesh(i);
	}

	m_ShaderObject->UnBind();
}

void Object3DEntity::RenderDisneyMode()
{
	m_ShaderObject->Bind();

	m_ShaderObject->SetTextureCubeMap("irradianceTex", m_SkyboxRenderer->GetIrradianceTexture()->GetID(), 6);
	m_ShaderObject->SetTextureCubeMap("prefilterTex", m_SkyboxRenderer->GetPreFilterTexture()->GetID(), 7);
	m_ShaderObject->SetTexture("brdfLUTTex", m_SkyboxRenderer->GetBRDFLUTTexture()->GetID(), 8);

	m_ShaderObject->SetTexture("ssaoTex", m_PostProcessRenderer->GetSSAOProcessedTexture(), 9);
	m_ShaderObject->SetInt("ssaoEnabled", m_PostProcessRenderer->IsSSAOEnabled() && m_PostProcessRenderer->IsEnabled());

	m_ShaderObject->SetTexture("shadowTex", m_Object3DRenderer->GetShadowDepthTexture(), 10);

	m_ShaderObject->SetVector3("cameraPos", m_MainCamera->GetPosition());

	m_ShaderObject->SetMat4("lightSpaceMatrix", m_Object3DRenderer->GetLightSpaceMatrix());
	m_ShaderObject->SetMat4("modelMatrix", m_ModelMatrix);

	for (int i = 0; i < m_MeshObject->GetSubMeshCount(); i++)
	{
		m_ShaderObject->SetBool("hasAlbedoTex", m_TexDiffuseSet[i] != -1);
		if (m_TexDiffuseSet[i] != -1) m_ShaderObject->SetTexture("albedoTex", m_TexDiffuseSet[i], 0);

		m_ShaderObject->SetBool("hasNormalTex", m_TexNormalSet[i] != -1);
		if (m_TexNormalSet[i] != -1) m_ShaderObject->SetTexture("normalTex", m_TexNormalSet[i], 1);

		m_ShaderObject->SetBool("hasMetallicTex", m_TexMetallicSet[i] != -1);
		if (m_TexMetallicSet[i] != -1) m_ShaderObject->SetTexture("metallicTex", m_TexMetallicSet[i], 2);

		m_ShaderObject->SetBool("hasRoughnessTex", m_TexRoughnessSet[i] != -1);
		if (m_TexRoughnessSet[i] != -1) m_ShaderObject->SetTexture("roughnessTex", m_TexRoughnessSet[i], 3);

		m_ShaderObject->SetBool("hasEmissiveTex", m_TexEmissionSet[i] != -1);
		if (m_TexEmissionSet[i] != -1) m_ShaderObject->SetTexture("emissiveTex", m_TexEmissionSet[i], 4);

		m_ShaderObject->SetBool("hasOcclusionTex", m_TexOcclusionSet[i] != -1);
		if (m_TexOcclusionSet[i] != -1) m_ShaderObject->SetTexture("occlusionTex", m_TexOcclusionSet[i], 5);

		Vector3 baseColor = Vector3(m_MaterialDisney.m_BaseColor.x, m_MaterialDisney.m_BaseColor.y, m_MaterialDisney.m_BaseColor.z);
		m_ShaderObject->SetVector3("u_BaseColor", baseColor);
		m_ShaderObject->SetFloat("u_Metallic", m_MaterialDisney.m_Metallic);
		m_ShaderObject->SetFloat("u_Subsurface", m_MaterialDisney.m_Subsurface);
		m_ShaderObject->SetFloat("u_Roughness", m_MaterialDisney.m_Roughness);
		m_ShaderObject->SetFloat("u_Specular", m_MaterialDisney.m_Specular);
		m_ShaderObject->SetFloat("u_SpecularTint", m_MaterialDisney.m_SpecularTint);
		m_ShaderObject->SetFloat("u_Anisotropic", m_MaterialDisney.m_Anisotropic);
		m_ShaderObject->SetFloat("u_Sheen", m_MaterialDisney.m_Sheen);
		m_ShaderObject->SetFloat("u_SheenTint", m_MaterialDisney.m_SheenTint);
		m_ShaderObject->SetFloat("u_ClearCoat", m_MaterialDisney.m_ClearCoat);
		m_ShaderObject->SetFloat("u_ClearCoatRoughness", m_MaterialDisney.m_ClearCoatRoughness);
		m_ShaderObject->SetFloat("u_Emission", m_MaterialDisney.m_Emission);

		m_MeshObject->DrawSubMesh(i);
	}

	m_ShaderObject->UnBind();
}

void Object3DEntity::RenderOrenNayarMode()
{
	m_ShaderObject->Bind();

	m_ShaderObject->SetTextureCubeMap("irradianceTex", m_SkyboxRenderer->GetIrradianceTexture()->GetID(), 5);

	m_ShaderObject->SetTexture("ssaoTex", m_PostProcessRenderer->GetSSAOProcessedTexture(), 6);
	m_ShaderObject->SetInt("ssaoEnabled", m_PostProcessRenderer->IsSSAOEnabled() && m_PostProcessRenderer->IsEnabled());

	m_ShaderObject->SetTexture("shadowTex", m_Object3DRenderer->GetShadowDepthTexture(), 7);

	m_ShaderObject->SetVector3("cameraPos", m_MainCamera->GetPosition());

	m_ShaderObject->SetMat4("lightSpaceMatrix", m_Object3DRenderer->GetLightSpaceMatrix());
	m_ShaderObject->SetMat4("modelMatrix", m_ModelMatrix);

	for (int i = 0; i < m_MeshObject->GetSubMeshCount(); i++)
	{
		m_ShaderObject->SetBool("hasAlbedoTex", m_TexDiffuseSet[i] != -1);
		if (m_TexDiffuseSet[i] != -1) m_ShaderObject->SetTexture("albedoTex", m_TexDiffuseSet[i], 0);

		m_ShaderObject->SetBool("hasNormalTex", m_TexNormalSet[i] != -1);
		if (m_TexNormalSet[i] != -1) m_ShaderObject->SetTexture("normalTex", m_TexNormalSet[i], 1);

		m_ShaderObject->SetBool("hasRoughnessTex", m_TexRoughnessSet[i] != -1);
		if (m_TexRoughnessSet[i] != -1) m_ShaderObject->SetTexture("roughnessTex", m_TexRoughnessSet[i], 2);

		m_ShaderObject->SetBool("hasEmissiveTex", m_TexEmissionSet[i] != -1);
		if (m_TexEmissionSet[i] != -1) m_ShaderObject->SetTexture("emissiveTex", m_TexEmissionSet[i], 3);

		m_ShaderObject->SetBool("hasOcclusionTex", m_TexOcclusionSet[i] != -1);
		if (m_TexOcclusionSet[i] != -1) m_ShaderObject->SetTexture("occlusionTex", m_TexOcclusionSet[i], 4);

		Vector3 baseColor = Vector3(m_MaterialOrenNayar.m_BaseColor.x, m_MaterialOrenNayar.m_BaseColor.y, m_MaterialOrenNayar.m_BaseColor.z);
		m_ShaderObject->SetVector3("u_BaseColor", baseColor);
		m_ShaderObject->SetBool("u_SpecularEnabled", m_MaterialOrenNayar.m_SpecularEnabled);
		m_ShaderObject->SetInt("u_SpecularType", m_MaterialOrenNayar.m_SpecularType);
		m_ShaderObject->SetFloat("u_Roughness", m_MaterialOrenNayar.m_Roughness);
		m_ShaderObject->SetFloat("u_Emission", m_MaterialOrenNayar.m_Emission);

		m_MeshObject->DrawSubMesh(i);
	}

	m_ShaderObject->UnBind();
}

void Object3DEntity::RenderShaderProperties()
{
	switch (m_ShaderMode)
	{
		case 0:		//PBR Mode
		{
			ImGui::ColorEdit4("Base Color", (float*)&m_MaterialPBR.m_BaseColor);

			ImGui::DragFloat("Metallic", &m_MaterialPBR.m_Metallic, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Roughness", &m_MaterialPBR.m_Roughness, 0.01f, 0.0f, 1.0f);

			ImGui::DragFloat("Emission", &m_MaterialPBR.m_Emission, 0.01f, 0.0f, 5.0f);
			break;
		}

		case 1:		//Blinn Phong
		{
			ImGui::ColorEdit4("Base Color", (float*)&m_MaterialBlinnPhong.m_BaseColor);

			ImGui::DragFloat("Specular", &m_MaterialBlinnPhong.m_Specular, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("Specular Power", &m_MaterialBlinnPhong.m_Shininess, 1.0f, 0.0f, 256.0f);

			ImGui::DragFloat("Emission", &m_MaterialBlinnPhong.m_Emission, 0.01f, 0.0f, 5.0f);
			break;
		}

		case 2:		//Disney Mode
		{
			ImGui::ColorEdit4("Base Color", (float*)&m_MaterialDisney.m_BaseColor);

			ImGui::DragFloat("Metallic", &m_MaterialDisney.m_Metallic, 0.01f, 0.0f, 1.0f);

			ImGui::DragFloat("Subsurface", &m_MaterialDisney.m_Subsurface, 0.01f, 0.0f, 1.0f);

			ImGui::DragFloat("Roughness", &m_MaterialDisney.m_Roughness, 0.01f, 0.0f, 1.0f);

			ImGui::DragFloat("Specular", &m_MaterialDisney.m_Specular, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Specular Tint", &m_MaterialDisney.m_SpecularTint, 0.01f, 0.0f, 1.0f);

			ImGui::DragFloat("Anisotropic", &m_MaterialDisney.m_Anisotropic, 0.01f, 0.0f, 1.0f);

			ImGui::DragFloat("Sheen", &m_MaterialDisney.m_Sheen, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Sheen Tint", &m_MaterialDisney.m_SheenTint, 0.01f, 0.0f, 1.0f);

			ImGui::DragFloat("Clear Coat", &m_MaterialDisney.m_ClearCoat, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Clear Coat Roughness", &m_MaterialDisney.m_ClearCoatRoughness, 0.01f, 0.0f, 1.0f);

			ImGui::DragFloat("Emission", &m_MaterialDisney.m_Emission, 0.01f, 0.0f, 5.0f);
			break;
		}

		case 3:		//Oren-Nayar Mode
		{
			ImGui::ColorEdit4("Base Color", (float*)&m_MaterialOrenNayar.m_BaseColor);

			ImGui::Checkbox("Enable Specular", &m_MaterialOrenNayar.m_SpecularEnabled);
			if (m_MaterialOrenNayar.m_SpecularEnabled)
				ImGui::Combo("Specular Type", &m_MaterialOrenNayar.m_SpecularType, "Gaussian\0Beckmann\0GGX");

			ImGui::DragFloat("Roughness", &m_MaterialOrenNayar.m_Roughness, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Emission", &m_MaterialOrenNayar.m_Emission, 0.01f, 0.0f, 5.0f);
			break;
		}
	}
}