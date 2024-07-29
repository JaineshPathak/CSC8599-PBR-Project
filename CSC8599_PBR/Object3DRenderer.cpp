#include "Object3DRenderer.h"
#include "Object3DEntity.h"
#include "Renderer.h"
#include "LookAtCamera.h"
#include "LightsManager.h"

#include <nclgl/FrameBuffer.h>
#include <nclgl/ProfilingManager.h>
#include <nclgl/DirectionalLight.h>

#include <imgui/imgui.h>

unsigned int Object3DRenderer::s_3DEntityIDs = 0;

Object3DRenderer::Object3DRenderer(const float& width, const float& height) : 
	m_Width(width), m_Height(height), 
	m_ShaderMode(0),
	m_Current3DEntityIndex(0),
	m_IsInitialized(false)
{
	m_MainCamera = Renderer::Get()->GetMainCamera();

	if (!InitShaders()) return;
	if (!InitBuffers()) return;
	if (!InitMeshes()) return;

	m_ShaderTex = std::shared_ptr<Texture>(new Texture(TEXTUREDIR + "Icons/Icon_Shader.png"));
	if (!m_ShaderTex->IsInitialized()) return;	

	m_3DEntities[0]->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

	m_3DEntitiesNamesList = new char* [m_3DEntitiesNames.size()];
	for (size_t i = 0; i < m_3DEntitiesNames.size(); i++)
		m_3DEntitiesNamesList[i] = (char*)m_3DEntitiesNames[i].c_str();

	m_MainCamera->SetLookAtDistance(m_3DEntities[m_Current3DEntityIndex]->GetLookDistance());

	ImGuiRenderer::Get()->RegisterItem(this);

	m_IsInitialized = true;
}

Object3DRenderer::~Object3DRenderer()
{
	delete[] m_3DEntitiesNamesList;
}

bool Object3DRenderer::InitShaders()
{
	m_PBRShader = std::shared_ptr<Shader>(new Shader("PBR/PBRTexturedVertex.glsl", "PBR/PBRTexturedFragment.glsl"));
	m_BlinnShader = std::shared_ptr<Shader>(new Shader("PBR/PBRTexturedVertex.glsl", "PBR/PBRTexturedFragmentBlinnPhong.glsl"));
	m_DisneyShader = std::shared_ptr<Shader>(new Shader("PBR/PBRTexturedVertex.glsl", "PBR/PBRTexturedFragmentDisney.glsl"));
	m_OrenNayarShader = std::shared_ptr<Shader>(new Shader("PBR/PBRTexturedVertex.glsl", "PBR/PBRTexturedFragmentOrenNayar.glsl"));

	m_DepthBufferShader = std::shared_ptr<Shader>(new Shader("PBR/PBRDepthBufferVert.glsl", "PBR/PBRDepthBufferFrag.glsl"));
	m_ShadowDepthBufferShader = std::shared_ptr<Shader>(new Shader("PBR/PBRShadowDepthBufferVert.glsl", "PBR/PBRDepthBufferFrag.glsl"));

	if (!m_PBRShader->LoadSuccess() || !m_BlinnShader->LoadSuccess() || !m_DisneyShader->LoadSuccess() || !m_OrenNayarShader->LoadSuccess() || !m_DepthBufferShader->LoadSuccess() || !m_ShadowDepthBufferShader->LoadSuccess())
	{
		std::cout << "ERROR: Object3DRenderer: Failed to load Shader" << std::endl;
		return false;
	}

	return true;
}

bool Object3DRenderer::InitMeshes()
{
	ProfilingManager::RecordTextureTimeStart();

	if(Add3DObject("Car", "Mesh_Car_MiniCooper.msh", "Mesh_Car_MiniCooper.mat", 4.0f) == nullptr) return false;
	if(Add3DObject("Helmet", "Mesh_SciFi_Helmet.msh", "Mesh_SciFi_Helmet.mat", 3.0f) == nullptr) return false;
	if(Add3DObject("Character", "Mesh_SciFi_Character.msh", "Mesh_SciFi_Character.mat", 1.65f) == nullptr) return false;

	m_3DPlatform = std::shared_ptr<Object3DEntity>(new Object3DEntity("Platform", "Mesh_Platform.msh", "Mesh_Platform.mat", "", "", 3.0f));
	if (m_3DPlatform == nullptr) return false;
	m_3DPlatform->SetObjectShader(m_PBRShader);

	ProfilingManager::RecordTextureTimeEnd();

	return true;
}

bool Object3DRenderer::InitBuffers()
{
	m_DepthFrameBuffer = std::shared_ptr<FrameBuffer>(new FrameBuffer((unsigned int)m_Width, (unsigned int)m_Height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 0));
	if (m_DepthFrameBuffer == nullptr) return false;

	ImGuiRenderer::Get()->AddObserver(m_DepthFrameBuffer);

	m_ShadowDepthFrameBuffer = std::shared_ptr<FrameBuffer>(new FrameBuffer(1024, 1024, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 0));
	if (m_ShadowDepthFrameBuffer == nullptr) return false;

	return true;
}

const unsigned int Object3DRenderer::GetDepthTexture() const
{
	return m_DepthFrameBuffer->GetDepthAttachmentTex();
}

const unsigned int Object3DRenderer::GetShadowDepthTexture() const
{
	return m_ShadowDepthFrameBuffer->GetDepthAttachmentTex();
}

std::shared_ptr<Object3DEntity> Object3DRenderer::Add3DObject(const std::string& objectName, const std::string& objectMeshFile, const std::string& objectMeshMaterialFile, const float& lookAtDistance)
{
	std::shared_ptr<Object3DEntity> entity = std::shared_ptr<Object3DEntity>(new Object3DEntity(objectName, objectMeshFile, objectMeshMaterialFile, "", "", lookAtDistance));
	if (entity != nullptr)
	{
		entity->SetObjectShader(m_PBRShader);

		m_3DEntities[s_3DEntityIDs++] = entity;		
		m_3DEntitiesNames.emplace_back(objectName);
	}

	return entity;
}

void Object3DRenderer::ChangeShaderMode(const int& newShaderMode)
{
	if (m_3DEntities.size() == 0) return;
	if (m_ShaderMode == newShaderMode) return;

	m_ShaderMode = newShaderMode;
	for (auto it = m_3DEntities.cbegin(); it != m_3DEntities.cend(); ++it)
	{
		switch (m_ShaderMode)
		{
			case 0: it->second->SetObjectShader(m_PBRShader);	break;
			case 1: it->second->SetObjectShader(m_BlinnShader); break;
			case 2: it->second->SetObjectShader(m_DisneyShader); break;
			case 3: it->second->SetObjectShader(m_OrenNayarShader); break;
			default:it->second->SetObjectShader(m_PBRShader);	break;
		}

		it->second->SetShaderMode(m_ShaderMode);
	}

	//For 3D Platform
	switch (m_ShaderMode)
	{
		case 0: m_3DPlatform->SetObjectShader(m_PBRShader);	break;
		case 1: m_3DPlatform->SetObjectShader(m_BlinnShader); break;
		case 2: m_3DPlatform->SetObjectShader(m_DisneyShader); break;
		case 3: m_3DPlatform->SetObjectShader(m_OrenNayarShader); break;
		default:m_3DPlatform->SetObjectShader(m_PBRShader);	break;
	}
	m_3DPlatform->SetShaderMode(m_ShaderMode);
}

void Object3DRenderer::OnObject3DChanged()
{
	if (m_MainCamera == nullptr) return;		
	m_MainCamera->SetLookAtDistance(m_3DEntities[m_Current3DEntityIndex]->GetLookDistance());
}

void Object3DRenderer::RenderUsualDepths()
{
	m_DepthFrameBuffer->Bind();
	m_DepthBufferShader->Bind();

	m_DepthBufferShader->SetMat4("modelMatrix", m_3DEntities[m_Current3DEntityIndex]->GetModelMatrix());

	glClear(GL_DEPTH_BUFFER_BIT);
	Draw();

	m_DepthBufferShader->UnBind();
	m_DepthFrameBuffer->Unbind();
}

void Object3DRenderer::RenderShadowDepths()
{
	m_ShadowDepthFrameBuffer->Bind();
	m_ShadowDepthBufferShader->Bind();

	std::shared_ptr<DirectionalLight> dirLight = LightsManager::Get()->GetDirectionalLight();

	float near_plane = 0.1f, far_plane = 10.0f;
	Matrix4 lightProjection = Matrix4::Orthographic(near_plane, far_plane, 10.0f, -10.0f, 10.0f, -10.0f);
	Matrix4 lightView = Matrix4::BuildViewMatrix(-dirLight->GetLightDir(), Vector3::ZERO, Vector3::UP);
	m_LightSpaceMatrix = lightProjection * lightView;

	m_ShadowDepthBufferShader->SetMat4("lightSpaceMatrix", m_LightSpaceMatrix);
	m_ShadowDepthBufferShader->SetMat4("modelMatrix", m_3DEntities[m_Current3DEntityIndex]->GetModelMatrix());

	//glCullFace(GL_FRONT);
	glDisable(GL_CULL_FACE);
	
	glClear(GL_DEPTH_BUFFER_BIT);
	Draw();
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	m_ShadowDepthBufferShader->UnBind();
	m_ShadowDepthFrameBuffer->Unbind();
}

void Object3DRenderer::Draw()
{
	m_3DEntities[m_Current3DEntityIndex]->Draw();
	m_3DPlatform->Draw();
}

void Object3DRenderer::RenderDepths() 
{
	RenderUsualDepths();
	RenderShadowDepths();
}

void Object3DRenderer::Render()
{
	m_3DEntities[m_Current3DEntityIndex]->Render();
	RenderPlatform();
}

void Object3DRenderer::RenderPlatform()
{
	m_3DPlatform->Render();
}

void Object3DRenderer::OnImGuiRender()
{
	if (ImGui::CollapsingHeader("3D Objects"))
	{
		ImGui::Indent();

		if (ImGui::Combo("Object Type", &m_Current3DEntityIndex, m_3DEntitiesNamesList, (int)m_3DEntities.size()))
			OnObject3DChanged();

		/*if (ImGui::Button("PBR Mode")) ChangeShaderMode(0);
		ImGui::SameLine();
		if (ImGui::Button("Blinn Mode")) ChangeShaderMode(1);
		ImGui::NewLine();
		if (ImGui::Button("Disney Mode")) ChangeShaderMode(2);
		ImGui::SameLine();
		if (ImGui::Button("Oren-Nayar Mode")) ChangeShaderMode(3);*/

		ImGui::Unindent();
	}

	ImGui::Begin("Shaders");
	const float PADDING = 16.0f;
	const float BUTTON_SIZE = 64.0f;
	float cellsize = BUTTON_SIZE + PADDING;

	float panelWidth = ImGui::GetContentRegionAvail().x;
	int columnsCount = (int)(panelWidth / cellsize);
	if (columnsCount < 1)
		columnsCount = 1;

	ImGui::Columns(columnsCount, 0, false);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	if (ImGui::ImageButton("PBR", (ImTextureID)m_ShaderTex->GetID(), {BUTTON_SIZE, BUTTON_SIZE}, {0, 1}, {1, 0}))
		ChangeShaderMode(0);
	ImGui::Text("PBR");

	ImGui::NextColumn();

	if (ImGui::ImageButton("Blinn-Phong", (ImTextureID)m_ShaderTex->GetID(), {BUTTON_SIZE, BUTTON_SIZE}, {0, 1}, {1, 0}))
		ChangeShaderMode(1);
	ImGui::Text("Blinn-Phong");

	ImGui::NextColumn();

	if (ImGui::ImageButton("Disney", (ImTextureID)m_ShaderTex->GetID(), {BUTTON_SIZE, BUTTON_SIZE}, {0, 1}, {1, 0}))
		ChangeShaderMode(2);
	ImGui::Text("Disney");

	ImGui::NextColumn();

	if (ImGui::ImageButton("Oren-Nayar", (ImTextureID)m_ShaderTex->GetID(), {BUTTON_SIZE, BUTTON_SIZE}, {0, 1}, {1, 0}))
		ChangeShaderMode(3);
	ImGui::Text("Oren-Nayar");
	ImGui::PopStyleColor();
	
	ImGui::Columns(1.0f);
	ImGui::End();

	ImGui::Begin("Shader Properties");
	m_3DEntities[m_Current3DEntityIndex]->RenderShaderProperties();
	ImGui::End();
}
