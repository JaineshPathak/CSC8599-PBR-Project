#include "SkyboxRenderer.h"
#include "Renderer.h"

#include <nclgl/Shader.h>
#include <nclgl/Texture.h>
#include <nclgl/TextureHDR.h>
#include <nclgl/TextureEnvCubeMap.h>
#include <nclgl/TextureCubeMap.h>
#include <nclgl/FrameBuffer.h>
#include <nclgl/FrameBufferFP.h>
#include <nclgl/FrameBufferHDR.h>
#include <nclgl/UniformBuffer.h>
#include <nclgl/ProfilingManager.h>

SkyboxRenderer::SkyboxRenderer() : m_SkyboxesIndexCurrent(0), m_Exposure(5.0f), m_Gamma(2.2f)
{
	if (!InitShaders()) { m_IsInitialized = false; return; }
	if (!InitBuffers()) { m_IsInitialized = false; return; }
	if (!InitTextures()) { m_IsInitialized = false; return; }

	m_SkyboxUBO = std::shared_ptr<UniformBuffer>(new UniformBuffer(sizeof(SkyboxData), nullptr, GL_DYNAMIC_DRAW, 4, 0));
	if(!m_SkyboxUBO->IsInitialized()) { m_IsInitialized = false; return; }
	
	m_SkyboxData = SkyboxData();
	m_SkyboxData.data = Vector4(m_Exposure, m_Gamma, 0.0f, 1.0f);
	BindSkyboxUBOData();

	m_CaptureProjection = Matrix4::Perspective(0.1f, 100.0f, 1.0f, 90.0f);

	m_CaptureViews[0] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::RIGHT, Vector3::DOWN);
	m_CaptureViews[1] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::LEFT, Vector3::DOWN);
	m_CaptureViews[2] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::UP, Vector3::BACK);
	m_CaptureViews[3] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::DOWN, Vector3::FORWARD);
	m_CaptureViews[4] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::BACK, Vector3::DOWN);
	m_CaptureViews[5] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::FORWARD, Vector3::DOWN);

	/*m_AlreadyCapturedCubeMap = false;
	m_AlreadyCapturedBRDFLUTMap = false;
	m_AlreadyCapturedIrradianceMap = false;
	m_AlreadyCapturedPreFilterMipMaps = false;*/

	ImGuiRenderer::Get()->RegisterItem(this);
	
	m_IsInitialized = true;
}

SkyboxRenderer::~SkyboxRenderer()
{
	delete[] m_SkyboxesNamesList;
}

bool SkyboxRenderer::InitShaders()
{
	m_CubeMapShader = std::shared_ptr<Shader>(new Shader("PBR/PBRSkyBox2Vertex.glsl", "PBR/PBRSkyBox2Fragment.glsl"));
	if (!m_CubeMapShader->LoadSuccess()) return false;

	m_EquiRect2CubeMapShader = std::shared_ptr<Shader>(new Shader("PBR/PBREquiRect2CubeMapVertex.glsl", "PBR/PBREquiRect2CubeMapFragment.glsl"));
	if (!m_EquiRect2CubeMapShader->LoadSuccess()) return false;

	m_IrradianceCubeMapShader = std::shared_ptr<Shader>(new Shader("PBR/PBREquiRect2CubeMapVertex.glsl", "PBR/PBRIrradianceCubeMapFragment.glsl"));
	if (!m_IrradianceCubeMapShader->LoadSuccess()) return false;

	m_PreFilterCubeMapShader = std::shared_ptr<Shader>(new Shader("PBR/PBREquiRect2CubeMapVertex.glsl", "PBR/PBRPreFilterConvolutionFragment.glsl"));
	if (!m_PreFilterCubeMapShader->LoadSuccess()) return false;

	m_BRDFIntegrateShader = std::shared_ptr<Shader>(new Shader("PBR/PBRBRDFConvolutionVertex.glsl", "PBR/PBRBRDFConvolutionFragment.glsl"));
	if (!m_BRDFIntegrateShader->LoadSuccess()) return false;

	return true;
}

bool SkyboxRenderer::InitBuffers()
{
	m_CaptureHDRFrameBuffer = std::shared_ptr<FrameBufferFP>(new FrameBufferFP(2048, 2048));
	if (m_CaptureHDRFrameBuffer == nullptr) return false;

	m_CaptureIrradianceFrameBuffer = std::shared_ptr<FrameBufferFP>(new FrameBufferFP(32, 32));
	if (m_CaptureIrradianceFrameBuffer == nullptr) return false;

	m_CapturePreFilterFrameBuffer = std::shared_ptr<FrameBufferHDR>(new FrameBufferHDR(128, 128));
	if (m_CapturePreFilterFrameBuffer == nullptr) return false;

	return true;
}

bool SkyboxRenderer::InitTextures()
{
	/*m_CubeMapTexture = std::shared_ptr<TextureCubeMap>(new TextureCubeMap(
		TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg"));
	if (!m_CubeMapTexture->IsInitialized()) return false;*/

	/*m_CubeMapHDRTexture = std::shared_ptr<TextureHDR>(new TextureHDR(TEXTUREDIR"HDR/solitude_night_2k.hdr"));
	if (!m_CubeMapHDRTexture->IsInitialized()) return false;

	m_CubeMapEnvTexture = std::shared_ptr<TextureEnvCubeMap>(new TextureEnvCubeMap(2048, 2048));
	if (!m_CubeMapEnvTexture->IsInitialized()) return false;

	m_CubeMapIrradianceTexture = std::shared_ptr<TextureEnvCubeMap>(new TextureEnvCubeMap(32, 32));
	if (!m_CubeMapIrradianceTexture->IsInitialized()) return false;

	m_CubeMapPreFilterTexture = std::shared_ptr<TextureEnvCubeMap>(new TextureEnvCubeMap(128, 128, true));
	if (!m_CubeMapPreFilterTexture->IsInitialized()) return false;

	m_BRDFLUTTexture = std::shared_ptr<Texture>(new Texture(512, 512, GL_RG16F, GL_RG));
	if (!m_BRDFLUTTexture->IsInitialized()) return false;*/

	AddSkyboxCubeMap("HDR/clarens_night_02_2k.hdr", "Clarens Night");
	AddSkyboxCubeMap("HDR/decor_shop_2k.hdr", "Interior Shop");
	AddSkyboxCubeMap("HDR/snowy_hillside_02_2k.hdr", "Snowy Hills");
	AddSkyboxCubeMap("HDR/pretville_cinema_2k.hdr", "Cinewall Hall");
	AddSkyboxCubeMap("HDR/solitude_night_2k.hdr", "Palace Night");

	m_SkyboxesNamesList = new char* [m_SkyBoxesNames.size()];
	for (size_t i = 0; i < m_SkyBoxesNames.size(); i++)	
		m_SkyboxesNamesList[i] = (char*)m_SkyBoxesNames[i].c_str();

	return true;
}

void SkyboxRenderer::AddSkyboxCubeMap(const std::string& fileName, const std::string& skyboxName)
{
	m_SkyboxesList.push_back(SkyboxCubeMap(TEXTUREDIR + fileName, skyboxName));
	m_SkyBoxesNames.push_back(skyboxName);
}

void SkyboxRenderer::CaptureHDRCubeMap()
{
	m_EquiRect2CubeMapShader->Bind();
	m_EquiRect2CubeMapShader->SetMat4("proj", m_CaptureProjection);
	m_EquiRect2CubeMapShader->SetTexture("equiRectTex", m_SkyboxesList[m_SkyboxesIndexCurrent].m_CubeMapHDRTexture->GetID(), 0);
	//m_EquiRect2CubeMapShader->SetTexture("equiRectTex", m_CubeMapHDRTexture->GetID(), 0);

	m_CaptureHDRFrameBuffer->Bind();
	for (unsigned int i = 0; i < 6; i++)
	{
		m_EquiRect2CubeMapShader->SetMat4("view", m_CaptureViews[i]);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_CubeMapEnvTexture->GetID(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_SkyboxesList[m_SkyboxesIndexCurrent].m_CubeMapEnvTexture->GetID(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Renderer::Get()->GetCubeMesh()->Draw();		
	}
	m_CaptureHDRFrameBuffer->Unbind();
	m_EquiRect2CubeMapShader->UnBind();
}

void SkyboxRenderer::CaptureIrradianceMap()
{
	m_IrradianceCubeMapShader->Bind();
	m_IrradianceCubeMapShader->SetMat4("proj", m_CaptureProjection);
	m_IrradianceCubeMapShader->SetTextureCubeMap("environmentHDRCubemap", m_SkyboxesList[m_SkyboxesIndexCurrent].m_CubeMapEnvTexture->GetID(), 0);
	//m_IrradianceCubeMapShader->SetTextureCubeMap("environmentHDRCubemap", m_CubeMapEnvTexture->GetID(), 0);

	m_CaptureIrradianceFrameBuffer->Bind();
	for (unsigned int i = 0; i < 6; i++)
	{
		m_IrradianceCubeMapShader->SetMat4("view", m_CaptureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_SkyboxesList[m_SkyboxesIndexCurrent].m_CubeMapIrradianceTexture->GetID(), 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_CubeMapIrradianceTexture->GetID(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Renderer::Get()->GetCubeMesh()->Draw();
	}
	m_CaptureIrradianceFrameBuffer->Unbind();
	m_IrradianceCubeMapShader->UnBind();
}

void SkyboxRenderer::CapturePreFilterMipMaps()
{
	m_PreFilterCubeMapShader->Bind();
	m_PreFilterCubeMapShader->SetMat4("proj", m_CaptureProjection);
	m_PreFilterCubeMapShader->SetTextureCubeMap("environmentHDRCubemap", m_SkyboxesList[m_SkyboxesIndexCurrent].m_CubeMapEnvTexture->GetID(), 0);
	//m_PreFilterCubeMapShader->SetTextureCubeMap("environmentHDRCubemap", m_CubeMapEnvTexture->GetID(), 0);

	m_CapturePreFilterFrameBuffer->Bind();
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipWidth = unsigned int(128 * std::pow(0.5, mip));
		unsigned int mipHeight = unsigned int(128 * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, m_CapturePreFilterFrameBuffer->GetRenderBufferID());
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		m_PreFilterCubeMapShader->SetFloat("roughnessStrength", roughness);

		for (unsigned int i = 0; i < 6; i++)
		{
			m_PreFilterCubeMapShader->SetMat4("view", m_CaptureViews[i]);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_CubeMapPreFilterTexture->GetID(), mip);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_SkyboxesList[m_SkyboxesIndexCurrent].m_CubeMapPreFilterTexture->GetID(), mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Renderer::Get()->GetCubeMesh()->Draw();
		}
	}

	m_CapturePreFilterFrameBuffer->Unbind();
	m_PreFilterCubeMapShader->UnBind();
}

void SkyboxRenderer::CaptureBRDFLUTMap()
{
	m_CapturePreFilterFrameBuffer->Bind();

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BRDFLUTTexture->GetID(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_SkyboxesList[m_SkyboxesIndexCurrent].m_BRDFLUTTexture->GetID(), 0);
	glViewport(0, 0, 512, 512);

	m_BRDFIntegrateShader->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Renderer::Get()->GetQuadMesh()->Draw();
	m_BRDFIntegrateShader->UnBind();

	m_CapturePreFilterFrameBuffer->Unbind();
}

void SkyboxRenderer::Render()
{
#pragma region For Single Maps
	/*if (!m_AlreadyCapturedCubeMap)
{
	m_AlreadyCapturedCubeMap = true;
	CaptureHDRCubeMap();
}
if (!m_AlreadyCapturedIrradianceMap)
{
	m_AlreadyCapturedIrradianceMap = true;
	CaptureIrradianceMap();
}
if (!m_AlreadyCapturedPreFilterMipMaps)
{
	m_AlreadyCapturedPreFilterMipMaps = true;
	CapturePreFilterMipMaps();
}
if (!m_AlreadyCapturedBRDFLUTMap)
{
	m_AlreadyCapturedBRDFLUTMap = true;
	CaptureBRDFLUTMap();
}*/
#pragma endregion

#pragma region For Multiple Maps
	/*if (!m_SkyboxesList[m_SkyboxesIndexCurrent].m_AlreadyCapturedCubeMap)
	{
		m_SkyboxesList[m_SkyboxesIndexCurrent].m_AlreadyCapturedCubeMap = true;
		CaptureHDRCubeMap();
	}
	if (!m_SkyboxesList[m_SkyboxesIndexCurrent].m_AlreadyCapturedIrradianceMap)
	{
		m_SkyboxesList[m_SkyboxesIndexCurrent].m_AlreadyCapturedIrradianceMap = true;
		CaptureIrradianceMap();
	}
	if (!m_SkyboxesList[m_SkyboxesIndexCurrent].m_AlreadyCapturedPreFilterMipMaps)
	{
		m_SkyboxesList[m_SkyboxesIndexCurrent].m_AlreadyCapturedPreFilterMipMaps = true;
		CapturePreFilterMipMaps();
	}
	if (!m_SkyboxesList[m_SkyboxesIndexCurrent].m_AlreadyCapturedBRDFLUTMap)
	{
		m_SkyboxesList[m_SkyboxesIndexCurrent].m_AlreadyCapturedBRDFLUTMap = true;
		CaptureBRDFLUTMap();
	}*/
#pragma endregion

	if (!m_CheckedAllSkyboxes)
	{
		m_CheckedAllSkyboxes = true;
		CheckAllSkyboxCaptures();
	}

	RenderSkybox();
}

void SkyboxRenderer::RenderSkybox()
{
	glDepthFunc(GL_LEQUAL);

	m_CubeMapShader->Bind();
	//m_CubeMapShader->SetTextureCubeMap("cubeTex", m_CubeMapIrradianceTexture->GetID(), 0);
	//m_CubeMapShader->SetTextureCubeMap("cubeTex", m_CubeMapEnvTexture->GetID(), 0);
	//m_CubeMapShader->SetTextureCubeMap("cubeTex", m_CubeMapEnvTexture->GetID(), 0);
	m_CubeMapShader->SetTextureCubeMap("cubeTex", m_SkyboxesList[m_SkyboxesIndexCurrent].m_CubeMapEnvTexture->GetID(), 0);
	Renderer::Get()->GetCubeMesh()->Draw();
	m_CubeMapShader->UnBind();

	glDepthFunc(GL_LESS);
}

void SkyboxRenderer::BindSkyboxUBOData()
{
	m_SkyboxUBO->Bind();
	m_SkyboxUBO->BindSubData(0, sizeof(SkyboxData), &m_SkyboxData);
	m_SkyboxUBO->Unbind();
}

void SkyboxRenderer::CheckAllSkyboxCaptures()
{
	if ((int)m_SkyboxesList.size() == 0) return;

	ProfilingManager::RecordSkyboxCaptureTimeStart();
	int i = 0;
	m_SkyboxesIndexCurrent = i;
	for (auto skybox : m_SkyboxesList)
	{
		if (!skybox.m_AlreadyCapturedCubeMap)
		{
			skybox.m_AlreadyCapturedCubeMap = true;
			CaptureHDRCubeMap();
		}
		if (!skybox.m_AlreadyCapturedIrradianceMap)
		{
			skybox.m_AlreadyCapturedIrradianceMap = true;
			CaptureIrradianceMap();
		}
		if (!skybox.m_AlreadyCapturedPreFilterMipMaps)
		{
			skybox.m_AlreadyCapturedPreFilterMipMaps = true;
			CapturePreFilterMipMaps();
		}
		if (!skybox.m_AlreadyCapturedBRDFLUTMap)
		{
			skybox.m_AlreadyCapturedBRDFLUTMap = true;
			CaptureBRDFLUTMap();
		}

		i++;
		m_SkyboxesIndexCurrent = i;
	}

	m_SkyboxesIndexCurrent = 0;
	ProfilingManager::RecordSkyboxCaptureTimeEnd();
}

void SkyboxRenderer::OnSkyboxDataChanged()
{
	m_SkyboxData.data = Vector4(m_Exposure, m_Gamma, 0.0f, 1.0f);
	BindSkyboxUBOData();
}

void SkyboxRenderer::OnImGuiRender()
{
	if (ImGui::CollapsingHeader("Skybox"))
	{
		ImGui::Indent();

		ImGui::Combo("Skybox Type", &m_SkyboxesIndexCurrent, m_SkyboxesNamesList, (int)m_SkyBoxesNames.size());

		float exposure = m_Exposure;
		if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 8.0f))
		{
			m_Exposure = exposure;
			OnSkyboxDataChanged();
		}

		float gamma = m_Gamma;
		if (ImGui::SliderFloat("Gamma", &gamma, 0.1f, 8.0f))
		{
			m_Gamma = gamma;
			OnSkyboxDataChanged();
		}
		ImGui::Unindent();
	}
}

SkyboxCubeMap::SkyboxCubeMap(const std::string& fileName, const std::string& skyboxName) : m_SkyboxName(skyboxName), m_SkyboxFileName(fileName)
{
	size_t lastDot = m_SkyboxFileName.find_last_of(".");
	m_SkyboxFileNameNoExt = (lastDot == std::string::npos) ? m_SkyboxFileName : m_SkyboxFileName.substr(0, lastDot);

	m_CubeMapHDRTexture = std::shared_ptr<TextureHDR>(new TextureHDR(/*TEXTUREDIR + */fileName));
	if (!m_CubeMapHDRTexture->IsInitialized()) return;

	m_CubeMapEnvTexture = std::shared_ptr<TextureEnvCubeMap>(new TextureEnvCubeMap(2048, 2048));
	if (!m_CubeMapEnvTexture->IsInitialized()) return;

	m_CubeMapIrradianceTexture = std::shared_ptr<TextureEnvCubeMap>(new TextureEnvCubeMap(32, 32));
	if (!m_CubeMapIrradianceTexture->IsInitialized()) return;

	m_CubeMapPreFilterTexture = std::shared_ptr<TextureEnvCubeMap>(new TextureEnvCubeMap(128, 128, true));
	if (!m_CubeMapPreFilterTexture->IsInitialized()) return;

	m_BRDFLUTTexture = std::shared_ptr<Texture>(new Texture(512, 512, GL_RG16F, GL_RG, GL_FLOAT));
	if (!m_BRDFLUTTexture->IsInitialized()) return;

	m_AlreadyCapturedCubeMap = false;
	m_AlreadyCapturedBRDFLUTMap = false;
	m_AlreadyCapturedIrradianceMap = false;
	m_AlreadyCapturedPreFilterMipMaps = false;
}