#include "PostProcessRenderer.h"
#include "Renderer.h"

#include <nclgl/ProfilingManager.h>

PostProcessRenderer::PostProcessRenderer(const unsigned int& sizeX, const unsigned int& sizeY) :
	m_IsEnabled(true),
	m_Width(sizeX),
	m_Height(sizeY)
{
	m_PostFinalShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostBloomVert.glsl", "PostProcess/PostFinalFrag.glsl"));
	if (!m_PostFinalShader->LoadSuccess()) return;

	m_FinalFBO = std::shared_ptr<FrameBuffer>(new FrameBuffer(m_Width, m_Height, GL_RGB16F, GL_RGB, GL_FLOAT, 1));
	m_FinalFBO->RemoveDepthAttachment();
	m_FinalFBO->SetDeleteDepthTextureStatus(true);

	ImGuiRenderer::Get()->AddObserver(m_FinalFBO);

	AddPostProcessEffect(std::shared_ptr<PostProcessEffect>(new PostProcessSSAO(m_Width, m_Height)));
	AddPostProcessEffect(std::shared_ptr<PostProcessEffect>(new PostProcessBloom(m_Width, m_Height)));
	AddPostProcessEffect(std::shared_ptr<PostProcessEffect>(new PostProcessVignette(m_Width, m_Height)));
	AddPostProcessEffect(std::shared_ptr<PostProcessEffect>(new PostProcessInvertColor(m_Width, m_Height)));

	m_IsInitialized = true;
}

PostProcessRenderer::~PostProcessRenderer()
{
	m_IsInitialized = false;
}

const unsigned int PostProcessRenderer::GetMidFinalTexture() const
{
	//return m_FinalTextureID;
	return m_FinalFBO->GetColorAttachmentTex();
}

void PostProcessRenderer::RenderSSAOPass(const unsigned int& depthTextureID)
{
	if (!m_IsEnabled) return;

	//CheckWindowSize();
	m_MidPostEffects[0]->Render(0, depthTextureID);
}

void PostProcessRenderer::RenderPrePass(const unsigned int& srcTexture, const unsigned int& depthTextureID)
{
	if (!m_IsEnabled) return;

	ProfilingManager::RecordPostProcessTimeStart();
	//CheckWindowSize();
	FillPreActivePostEffects();
	RenderPreActivePostEffects(srcTexture, depthTextureID);

	m_ActivePrePassPostEffects.clear();
}

void PostProcessRenderer::Render(const unsigned int& srcTexture, const unsigned int& depthTextureID)
{
	if (!m_IsEnabled) return;
	
	//CheckWindowSize();
	FillMidActivePostEffects();
	RenderMidActivePostEffects(srcTexture, depthTextureID);
	RenderMidFinalPostEffect();

	m_ActiveMidPostEffects.clear();

	RenderLastPass(GetMidFinalTexture(), depthTextureID);
	ProfilingManager::RecordPostProcessTimeEnd();
}

void PostProcessRenderer::RenderLastPass(const unsigned int& srcTexture, const unsigned int& depthTextureID)
{
	if (!m_IsEnabled) return;

	//CheckWindowSize();
	FillLastActivePostEffects();
	RenderLastActivePostEffects(srcTexture, depthTextureID);

	m_ActiveLastPassPostEffects.clear();
}

bool PostProcessRenderer::GetEnableStatus()
{
	return m_IsEnabled && ((int)m_ActiveMidPostEffects.size() > 0 || (int)m_ActiveLastPassPostEffects.size() > 0);
}

#pragma region Pre Pass Post Effects
void PostProcessRenderer::FillPreActivePostEffects()
{
	if ((int)m_PrePassPostEffects.size() == 0) return;

	for (int i = 0; i < (int)m_PrePassPostEffects.size(); i++)
	{
		if (m_PrePassPostEffects[i]->IsEnabled())
			m_ActivePrePassPostEffects.emplace_back(m_PrePassPostEffects[i]);
	}
}

void PostProcessRenderer::RenderPreActivePostEffects(const unsigned int& srcTexture, const unsigned int& depthTextureID)
{
	if ((int)m_ActivePrePassPostEffects.size() == 0)
	{
		m_PreFinalTextureID = srcTexture;
		return;
	}

	for (int i = 0; i < (int)m_ActivePrePassPostEffects.size(); i++)
	{
		PostProcessEffect& effect = *m_ActivePrePassPostEffects[i];
		effect.Render(i == 0 ? srcTexture : m_ActivePrePassPostEffects[i - 1]->GetProcessedTexture(), depthTextureID);
	}

	m_PreFinalTextureID = m_ActivePrePassPostEffects[m_ActivePrePassPostEffects.size() - 1]->GetProcessedTexture();
}
#pragma endregion


#pragma region Mid Pass Post Effects
void PostProcessRenderer::FillMidActivePostEffects()
{
	if ((int)m_MidPostEffects.size() == 0) return;

	for (int i = 0; i < (int)m_MidPostEffects.size(); i++)
	{
		if (m_MidPostEffects[i]->IsEnabled())
			m_ActiveMidPostEffects.emplace_back(m_MidPostEffects[i]);
	}
}

void PostProcessRenderer::RenderMidActivePostEffects(const unsigned int& srcTexture, const unsigned int& depthTextureID)
{
	if ((int)m_ActiveMidPostEffects.size() == 0)
	{
		m_FinalTextureID = srcTexture;
		return;
	}

	for (int i = 0; i < (int)m_ActiveMidPostEffects.size(); i++)
	{
		PostProcessEffect& effect = *m_ActiveMidPostEffects[i];
		effect.Render(i == 0 ? srcTexture : m_ActiveMidPostEffects[i - 1]->GetProcessedTexture(), depthTextureID);
	}

	m_FinalTextureID = m_ActiveMidPostEffects[m_ActiveMidPostEffects.size() - 1]->GetProcessedTexture();
}

void PostProcessRenderer::RenderMidFinalPostEffect()
{
	m_FinalFBO->Bind();
	m_PostFinalShader->Bind();
	
	m_PostFinalShader->SetInt("isPostProcessEnabled", GetEnableStatus());
	m_PostFinalShader->SetTexture("srcTexture", Renderer::Get()->GetGlobalFrameBuffer()->GetColorAttachmentTex(0), 0);
	m_PostFinalShader->SetTexture("postProcessTexture", m_FinalTextureID, 1);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	Renderer::Get()->GetQuadMesh()->Draw();

	m_PostFinalShader->UnBind();
	m_FinalFBO->Unbind();
}
#pragma endregion


#pragma region Last Pass Post Effects
void PostProcessRenderer::FillLastActivePostEffects()
{
	if ((int)m_LastPassPostEffects.size() == 0) return;

	for (int i = 0; i < (int)m_LastPassPostEffects.size(); i++)
	{
		if (m_LastPassPostEffects[i]->IsEnabled())
			m_ActiveLastPassPostEffects.emplace_back(m_LastPassPostEffects[i]);
	}
}

void PostProcessRenderer::RenderLastActivePostEffects(const unsigned int& srcTexture, const unsigned int& depthTextureID)
{
	if ((int)m_ActiveLastPassPostEffects.size() == 0)
	{
		m_LastFinalTextureID = srcTexture;
		return;
	}

	for (int i = 0; i < (int)m_ActiveLastPassPostEffects.size(); i++)
	{
		PostProcessEffect& effect = *m_ActiveLastPassPostEffects[i];
		effect.Render(i == 0 ? srcTexture : m_ActiveLastPassPostEffects[i - 1]->GetProcessedTexture(), depthTextureID);
	}

	m_LastFinalTextureID = m_ActiveLastPassPostEffects[m_ActiveLastPassPostEffects.size() - 1]->GetProcessedTexture();
}
#pragma endregion


void PostProcessRenderer::AddPostProcessEffect(std::shared_ptr<PostProcessEffect> effect)
{
	switch (effect->GetPostEffectType())
	{
		case EPostEffectType::Type_None: break;
		case EPostEffectType::Type_PrePass: m_PrePassPostEffects.emplace_back(effect); break;
		case EPostEffectType::Type_MidPass: m_MidPostEffects.emplace_back(effect); break;
		case EPostEffectType::Type_LastPass: m_LastPassPostEffects.emplace_back(effect); break;
	}

	m_AllPostEffects.emplace_back(effect);
}