#include "Renderer.h"
#include "LookAtCamera.h"
#include "ImGuiRenderer.h"
#include "LightsManager.h"
#include "SkyboxRenderer.h"
#include "PostProcessRenderer.h"
#include "Object3DRenderer.h"

#include <nclgl/Texture.h>
#include <nclgl/TextureHDR.h>
#include <nclgl/TextureEnvCubeMap.h>
#include <nclgl/TextureCubeMap.h>
#include <nclgl/FrameBuffer.h>
#include <nclgl/FrameBufferFP.h>
#include <nclgl/FrameBufferHDR.h>
#include <nclgl/UniformBuffer.h>
#include <nclgl/MeshMaterial.h>
#include <nclgl/ProfilingManager.h>
#include <stb_image/stb_image.h>

#if _DEBUG
#include <iostream>
#define Log(x) std::cout << x << std::endl
#endif

Renderer* Renderer::m_Renderer = nullptr;

Renderer::Renderer(Window& parent) : m_WindowParent(parent), OGLRenderer(parent)
{
	ProfilingManager::RecordStartupTimeStart();

	m_Renderer = this;

	init = Initialize();
	if (!init) return;

#if _DEBUG
	Log("Main Renderer: Everything is Initialised! Good To Go!");
#endif

	ProfilingManager::RecordStartupTimeEnd();
}

bool Renderer::Initialize()
{
	if (!InitImGui())				return false;
	if (!InitCamera())				return false;
	if (!InitShaders())				return false;
	if (!InitBuffers())				return false;
	if (!InitLights())				return false;
	if (!InitMesh())				return false;
	if (!InitPostProcessor())		return false;

	SetupGLParameters();
	
	return true;
}

bool Renderer::InitImGui()
{
	m_ImGuiRenderer = std::shared_ptr<ImGuiRenderer>(new ImGuiRenderer(m_WindowParent));
	return m_ImGuiRenderer->IsInitialised();
}

bool Renderer::InitCamera()
{
	m_MainCamera = std::shared_ptr<LookAtCamera>(new LookAtCamera(Vector3(0, 1.0f, 3.0f), Vector3(0, 0, 0)));
	m_MainCamera->SetLookAtDistance(m_MainCamera->GetPosition().z - 0.0f + 1.0f);
	m_MainCamera->SetLookAtPosition(Vector3(0, 1.0f, 0));
	return m_MainCamera != nullptr;
}

bool Renderer::InitShaders()
{
	/*
	m_PositionBufferShader = std::shared_ptr<Shader>(new Shader("PBR/PBRPositionBufferVert.glsl", "PBR/PBRPositionBufferFrag.glsl"));
	if (!m_PositionBufferShader->LoadSuccess()) return false;

	m_NormalsBufferShader = std::shared_ptr<Shader>(new Shader("PBR/PBRNormalBufferVert.glsl", "PBR/PBRNormalBufferFrag.glsl"));
	if (!m_NormalsBufferShader->LoadSuccess()) return false;
	*/

	m_CombinedShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostBloomVert.glsl", "PostProcess/PostFinalFrag.glsl"));
	if (!m_CombinedShader->LoadSuccess()) return false;

	return true;
}

bool Renderer::InitBuffers()
{
	float w = m_WindowParent.GetScreenSize().x;
	float h = m_WindowParent.GetScreenSize().y;

	m_GlobalFrameBuffer = std::shared_ptr<FrameBuffer>(new FrameBuffer((unsigned int)w, (unsigned int)h, GL_RGBA16F, GL_RGBA, GL_FLOAT, 2));
	if (m_GlobalFrameBuffer == nullptr) return false;
	m_ImGuiRenderer->AddObserver(m_GlobalFrameBuffer);

	/*
	m_PositionFrameBuffer = std::shared_ptr<FrameBuffer>(new FrameBuffer((unsigned int)w, (unsigned int)h, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 1));
	if (m_PositionFrameBuffer == nullptr) return false;
	m_PositionFrameBuffer->RemoveDepthAttachment();

	m_NormalsFrameBuffer = std::shared_ptr<FrameBuffer>(new FrameBuffer((unsigned int)w, (unsigned int)h, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 1));
	if (m_NormalsFrameBuffer == nullptr) return false;
	m_NormalsFrameBuffer->RemoveDepthAttachment();
	*/

	m_MatricesUBO = std::shared_ptr<UniformBuffer>(new UniformBuffer(2 * sizeof(Matrix4), NULL, GL_STATIC_DRAW, 0, 0));
	if (!m_MatricesUBO->IsInitialized()) return false;

	return true;
}

bool Renderer::InitLights()
{
	m_LightsManager = std::shared_ptr<LightsManager>(new LightsManager());
	if(!m_LightsManager->IsInitialized()) return false;

	m_SkyboxRenderer = std::shared_ptr<SkyboxRenderer>(new SkyboxRenderer());
	if (!m_SkyboxRenderer->IsInitialized()) return false;

	m_Object3DRenderer = std::shared_ptr<Object3DRenderer>(new Object3DRenderer((float)width, (float)height));
	if (!m_Object3DRenderer->IsInitialized()) return false;

	return true;
}

bool Renderer::InitMesh()
{
	m_QuadMesh = std::shared_ptr<Mesh>(Mesh::GenerateQuad());
	if (m_QuadMesh == nullptr) return false;

	m_CubeMesh = std::shared_ptr<Mesh>(Mesh::GenerateCube());
	if (m_CubeMesh == nullptr) return false;

	return true;
}

#pragma region Helmet Stuffs
/*m_HelmetTextureAlbedo = std::shared_ptr<Texture>(new Texture(TEXTUREDIR"Helmet/Helmet_BaseColor_sRGB.png"));
if (!m_HelmetTextureAlbedo->IsInitialized()) return false;

m_HelmetTextureNormal = std::shared_ptr<Texture>(new Texture(TEXTUREDIR"Helmet/Helmet_Normal_Raw.png"));
if (!m_HelmetTextureNormal->IsInitialized()) return false;

m_HelmetTextureMetallic = std::shared_ptr<Texture>(new Texture(TEXTUREDIR"Helmet/Helmet_Metallic_Raw.png", GL_RED, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, true));
if (!m_HelmetTextureMetallic->IsInitialized()) return false;

m_HelmetTextureRoughness = std::shared_ptr<Texture>(new Texture(TEXTUREDIR"Helmet/Helmet_Roughness_Raw.png", GL_RED, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, true));
if (!m_HelmetTextureRoughness->IsInitialized()) return false;

m_HelmetTextureEmissive = std::shared_ptr<Texture>(new Texture(TEXTUREDIR"Helmet/Helmet_Emissive_sRGB.png"));
if (!m_HelmetTextureEmissive->IsInitialized()) return false;*/
#pragma endregion

bool Renderer::InitPostProcessor()
{
	m_PostProcessRenderer = std::shared_ptr<PostProcessRenderer>(new PostProcessRenderer((unsigned int)width, (unsigned int)height));
	if (!m_PostProcessRenderer->IsInitialized()) return false;

	return true;
}

void Renderer::SetupGLParameters()
{
	m_MainCamera->SetAspectRatio((float)width, (float)height); 
	projMatrix = m_MainCamera->GetProjectionMatrix();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void Renderer::HandleInputs(float dt)
{	
	if (ImGui::GetIO().MouseClicked[1])
	{
		m_showCursor = !m_showCursor;
		ImGui::GetIO().MouseDrawCursor = m_showCursor;
		m_WindowParent.LockMouseToWindow(!m_showCursor);
	}

	if (ImGuiRenderer::Get()->IsMouseOverScene() && ImGui::GetIO().MouseClicked[0])
	{
		ImGui::GetIO().MouseDrawCursor = m_showCursor;
		m_WindowParent.LockMouseToWindow(true);		
	}
	else if (ImGui::GetIO().MouseReleased[0])
	{
		m_showCursor = true;
		ImGui::GetIO().MouseDrawCursor = m_showCursor;
		m_WindowParent.LockMouseToWindow(false);
	}
}

void Renderer::HandleUBOData()
{
	//Start Offset from 0, size = 64
	//Start Offset from 65, size = 64

	m_MatricesUBO->Bind();
	m_MatricesUBO->BindSubData(0, sizeof(Matrix4), m_MainCamera->GetProjectionMatrix().values);
	m_MatricesUBO->BindSubData(sizeof(Matrix4), sizeof(Matrix4), m_MainCamera->GetViewMatrix().values);
	m_MatricesUBO->Unbind();

	// Removed as Lights UBO Data will only be changed when there is any change in the Light Properties. No need for Polling
	// See LightsManager.cpp
	//m_LightsManager->BindLightUBOData();
}

void Renderer::RenderScene()
{	
	//Render the Depths
	m_Object3DRenderer->RenderDepths();

	//----------------------------------------------------------------------------

	//SSAO Pass
	//if (m_PostProcessRenderer != nullptr && m_PostProcessRenderer->IsEnabled() && m_PostProcessRenderer->IsSSAOEnabled())
		//m_PostProcessRenderer->RenderSSAOPass(m_Object3DRenderer->GetDepthTexture());
	if (m_PostProcessRenderer != nullptr && m_PostProcessRenderer->IsEnabled())
		m_PostProcessRenderer->RenderPrePass(0, m_Object3DRenderer->GetDepthTexture());

	//----------------------------------------------------------------------------

	/*
	//Render the Positions
	m_PositionFrameBuffer->Bind();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_PositionBufferShader->Bind();
	m_PositionBufferShader->SetMat4("modelMatrix", modelMatrix);

	for (int i = 0; i < m_HelmetMesh->GetSubMeshCount(); i++)
		m_HelmetMesh->DrawSubMesh(i);
	
	m_PositionBufferShader->UnBind();
	m_PositionFrameBuffer->Unbind();

	//----------------------------------------------------------------------------

	//Render the Normals
	m_NormalsFrameBuffer->Bind();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_NormalsBufferShader->Bind();
	m_NormalsBufferShader->SetTexture("normalTex", m_HelmetTextureNormal->GetID(), 0);
	m_NormalsBufferShader->SetMat4("modelMatrix", modelMatrix);

	for (int i = 0; i < m_HelmetMesh->GetSubMeshCount(); i++)
		m_HelmetMesh->DrawSubMesh(i);

	m_NormalsBufferShader->UnBind();
	m_NormalsFrameBuffer->Unbind();

	//----------------------------------------------------------------------------	
	*/
	m_GlobalFrameBuffer->Bind();	
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	HandleUBOData();
	m_Object3DRenderer->Render();
	m_SkyboxRenderer->Render();
	m_LightsManager->Render();

	m_GlobalFrameBuffer->Unbind();

	m_PostProcessRenderer->Render(m_GlobalFrameBuffer->GetColorAttachmentTex(1), m_Object3DRenderer->GetDepthTexture());
	RenderImGui();
	
	/*
	//----------------------------------------------------------------------------
	// TESTING AREA
	
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_CombinedShader->Bind();	
	m_CombinedShader->SetTexture("diffuseTex", m_GlobalFrameBuffer->GetColorAttachmentTex(), 0);
	m_QuadMesh->Draw();
	m_CombinedShader->UnBind();
	//----------------------------------------------------------------------------
	*/
}

void Renderer::RenderImGui()
{
	if (m_ImGuiRenderer == nullptr)
		return;

	ProfilingManager::RecordGUITimeStart();
	m_ImGuiRenderer->Render();
	ProfilingManager::RecordGUITimeEnd();
}

void Renderer::UpdateScene(float dt)
{
	HandleInputs(dt);

	if (m_MainCamera != nullptr)
	{
		m_MainCamera->UpdateCamera(dt);
		m_MainCamera->CalcViewMatrix();
	}

	ProfilingManager::Update();
}