#include "PostProcessBloom.h"

PostProcessBloom::PostProcessBloom(const unsigned int& sizeX, const unsigned int& sizeY) : 
	PostProcessEffect(sizeX, sizeY),
	m_BloomIterations(5),
	m_EnableBloomDebug(false),
	m_BloomFilterRadius(0.005f),
	m_BrightnessThreshold(1.0f),
	m_BrightnessSoftThreshold(0.5f),
	m_BloomStrength(1.0f),
	m_BloomTint(Vector4::WHITE),
	m_BloomTintStrength(1.0f),
	m_EnableDirtMask(false),
	m_DirtMaskStrength(1.0f)
{
	m_PostEffectType = EPostEffectType::Type_MidPass;

	if (!InitShaders()) return;
	if (!InitTextures()) return;

	//m_BloomFBO.~FrameBufferBloom();
	//new(&m_BloomFBO) FrameBufferBloom(m_WidthI, m_HeightI, m_BloomIterations);	
	m_BloomFBO = std::shared_ptr<FrameBufferBloom>(new FrameBufferBloom(m_WidthI, m_HeightI, m_BloomIterations));

	m_BrightenFBO = std::shared_ptr<FrameBuffer>(new FrameBuffer(m_WidthI, m_HeightI, GL_RGB16F, GL_RGB, GL_FLOAT, 1));
	m_BrightenFBO->RemoveDepthAttachment();
	m_BrightenFBO->SetDeleteDepthTextureStatus(true);

	m_FinalFBO = std::shared_ptr<FrameBuffer>(new FrameBuffer(m_WidthI, m_HeightI, GL_RGB16F, GL_RGB, GL_FLOAT, 1));
	m_FinalFBO->RemoveDepthAttachment();
	m_FinalFBO->SetDeleteDepthTextureStatus(true);

	m_CombinedFBO = std::shared_ptr<FrameBuffer>(new FrameBuffer(m_WidthI, m_HeightI, GL_RGB16F, GL_RGB, GL_FLOAT, 1));
	m_CombinedFBO->RemoveDepthAttachment();
	m_CombinedFBO->SetDeleteDepthTextureStatus(true);

	//m_BrightenFBO.~FrameBuffer();
	//new(&m_BrightenFBO) FrameBuffer(m_WidthI, m_HeightI, GL_RGB16F, GL_RGB, GL_FLOAT, 1);
	//m_BrightenFBO.RemoveDepthAttachment();

	//m_FinalFBO.~FrameBuffer();
	//new(&m_FinalFBO) FrameBuffer(m_WidthI, m_HeightI, GL_RGB16F, GL_RGB, GL_FLOAT, 1);
	//m_FinalFBO.RemoveDepthAttachment();

	//m_CombinedFBO.~FrameBuffer();
	//new(&m_CombinedFBO) FrameBuffer(m_WidthI, m_HeightI, GL_RGB16F, GL_RGB, GL_FLOAT, 1);
	//m_CombinedFBO.RemoveDepthAttachment();

	ImGuiRenderer::Get()->AddObserver(m_BloomFBO);
	ImGuiRenderer::Get()->AddObserver(m_BrightenFBO);
	ImGuiRenderer::Get()->AddObserver(m_FinalFBO);
	ImGuiRenderer::Get()->AddObserver(m_CombinedFBO);
	ImGuiRenderer::Get()->RegisterPostProcessItem(this);

	m_IsEnabled = true;
}

PostProcessBloom::~PostProcessBloom()
{
	m_BloomFBO->Destroy();
	m_BrightenFBO->Destroy();
	m_FinalFBO->Destroy();
	m_CombinedFBO->Destroy();
}

bool PostProcessBloom::InitShaders()
{
	m_PostBloomBrightenShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostBloomVert.glsl", "PostProcess/PostBloomBrightenFrag.glsl"));
	if (!m_PostBloomBrightenShader->LoadSuccess()) return false;

	m_PostBloomDownSampleShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostBloomVert.glsl", "PostProcess/PostBloomDownSampleFrag.glsl"));
	if (!m_PostBloomDownSampleShader->LoadSuccess()) return false;

	m_PostBloomUpSampleShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostBloomVert.glsl", "PostProcess/PostBloomUpSampleFrag.glsl"));
	if (!m_PostBloomUpSampleShader->LoadSuccess()) return false;

	m_PostBloomFinalShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostBloomVert.glsl", "PostProcess/PostBloomFinalFrag.glsl"));
	if (!m_PostBloomFinalShader->LoadSuccess()) return false;

	m_PostBloomCombinedShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostBloomVert.glsl", "PostProcess/PostCombinedFinalFrag.glsl"));
	if (!m_PostBloomCombinedShader->LoadSuccess()) return false;

	m_PostBloomDownSampleShader->Bind();
	m_PostBloomDownSampleShader->SetInt("srcTexture", 0);
	m_PostBloomDownSampleShader->UnBind();

	m_PostBloomUpSampleShader->Bind();
	m_PostBloomUpSampleShader->SetInt("srcTexture", 0);
	m_PostBloomUpSampleShader->UnBind();

	return true;
}

bool PostProcessBloom::InitTextures()
{
	m_DirtMaskTexture = std::shared_ptr<Texture>(new Texture(TEXTUREDIR + "PostProcess/LensDirt02.png"));
	if (!m_DirtMaskTexture->IsInitialized()) return false;

	return true;
}

void PostProcessBloom::RenderBrightColors(unsigned int srcTexture)
{
	m_BrightenFBO->Bind();
	m_PostBloomBrightenShader->Bind();
	m_PostBloomBrightenShader->SetTexture("srcTexture", srcTexture, 0);
	m_PostBloomBrightenShader->SetFloat("brightnessThreshold", m_BrightnessThreshold);
	m_PostBloomBrightenShader->SetFloat("brightnessSoftThreshold", m_BrightnessSoftThreshold);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_QuadMesh->Draw();

	m_PostBloomBrightenShader->UnBind();
	m_BrightenFBO->Unbind();
}

void PostProcessBloom::RenderDownSamples(unsigned int srcTexture)
{
	const std::vector<BloomMip>& mipChain = m_BloomFBO->MipChain();

	m_PostBloomDownSampleShader->Bind();
	m_PostBloomDownSampleShader->SetVector2("srcResolution", m_SrcViewportSize);
	m_PostBloomDownSampleShader->SetTexture("srcTexture", srcTexture, 0);

	for (int i = 0; i < (int)mipChain.size(); i++)
	{
		const BloomMip& mip = mipChain[i];

		glViewport(0, 0, mip.sizeX, mip.sizeY);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

		m_QuadMesh->Draw();

		m_PostBloomDownSampleShader->SetVector2("srcResolution", mip.size);
		m_PostBloomDownSampleShader->SetTexture("srcTexture", mip.texture, 0);
	}

	m_PostBloomDownSampleShader->UnBind();
}

void PostProcessBloom::RenderUpSamples()
{
	const std::vector<BloomMip>& mipChain = m_BloomFBO->MipChain();

	m_PostBloomUpSampleShader->Bind();
	m_PostBloomUpSampleShader->SetFloat("filterRadius", m_BloomFilterRadius);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	for (int i = (int)mipChain.size() - 1; i > 0; i--)
	{
		const BloomMip& mip = mipChain[i];
		const BloomMip& nextMip = mipChain[i - 1];

		m_PostBloomUpSampleShader->SetTexture("srcTexture", mip.texture, 0);

		glViewport(0, 0, nextMip.sizeX, nextMip.sizeY);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.texture, 0);

		m_QuadMesh->Draw();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable(GL_BLEND);

	m_PostBloomUpSampleShader->UnBind();
}

void PostProcessBloom::RenderBloomTexture(unsigned int srcTexture)
{
	m_BloomFBO->Bind();

	RenderDownSamples(srcTexture);
	RenderUpSamples();

	m_BloomFBO->Unbind();

	glViewport(0, 0, m_WidthI, m_HeightI);
}

void PostProcessBloom::RenderBloomFinal()
{
	m_FinalFBO->Bind();
	m_PostBloomFinalShader->Bind();
	//m_PostBloomFinalShader->SetTexture("srcTexture", sourceTextureID, 0);
	m_PostBloomFinalShader->SetTexture("postProcessTexture", GetBloomTexture(), 0);
	m_PostBloomFinalShader->SetTexture("dirtMaskTexture", m_DirtMaskTexture->GetID(), 1);

	m_PostBloomFinalShader->SetFloat("bloomStrength", m_BloomStrength);
	m_PostBloomFinalShader->SetFloat("bloomTintStrength", m_BloomTintStrength);
	m_PostBloomFinalShader->SetVector4("bloomTint", m_BloomTint);

	m_PostBloomFinalShader->SetInt("enableDirtMask", m_EnableDirtMask);
	m_PostBloomFinalShader->SetFloat("dirtMaskStrength", m_DirtMaskStrength);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FinalFBO.GetColorAttachmentTex(), 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	m_QuadMesh->Draw();

	m_PostBloomFinalShader->UnBind();
	m_FinalFBO->Unbind();
}

void PostProcessBloom::RenderBloomCombined()
{
	m_CombinedFBO->Bind();
	m_PostBloomCombinedShader->Bind();

	m_PostBloomCombinedShader->SetTexture("srcTexture", m_SourceTextureID, 0);
	m_PostBloomCombinedShader->SetTexture("postTexture", m_FinalFBO->GetColorAttachmentTex(), 1);
	m_PostBloomCombinedShader->SetInt("combineType", 0);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	m_QuadMesh->Draw();

	m_PostBloomCombinedShader->UnBind();
	m_CombinedFBO->Unbind();
}

void PostProcessBloom::OnIterationsChanged()
{
	m_BloomFBO->Recalculate(m_BloomIterations);
}

unsigned int PostProcessBloom::GetBloomTexture(int index)
{
	return m_BloomFBO->MipChain()[index].texture;
}

const unsigned int PostProcessBloom::GetProcessedTexture() const
{
	return m_EnableBloomDebug ? m_BrightenFBO->GetColorAttachmentTex() : m_FinalFBO->GetColorAttachmentTex();
}

void PostProcessBloom::Render(const unsigned int& sourceTextureID, const unsigned int& depthTextureID)
{
	m_SourceTextureID = sourceTextureID;

	RenderBrightColors(sourceTextureID);
	RenderBloomTexture(m_BrightenFBO->GetColorAttachmentTex());
	RenderBloomFinal();
	//RenderBloomCombined();
}

void PostProcessBloom::OnImGuiRender()
{
	if (ImGui::CollapsingHeader("Bloom"))
	{
		ImGui::Indent();

		bool enableBloom = m_IsEnabled;
		if (ImGui::Checkbox("Enable", &enableBloom)) m_IsEnabled = enableBloom;

		bool enableDebug = m_EnableBloomDebug;
		if (ImGui::Checkbox("Debug", &enableDebug)) m_EnableBloomDebug = enableDebug;

		int iterations = m_BloomIterations;
		if (ImGui::SliderInt("Iterations", &iterations, 1, 8))
		{
			m_BloomIterations = iterations;
			OnIterationsChanged();
		}

		ImGui::Spacing();

		float filterRad = m_BloomFilterRadius;
		if (ImGui::DragFloat("Filter Radius", &filterRad, 0.001f, 0.001f, 1.0f)) m_BloomFilterRadius = filterRad;

		float bloomStrength = m_BloomStrength;
		if (ImGui::DragFloat("Strength", &bloomStrength, 0.01f, 0.0f, 100.0f)) m_BloomStrength = bloomStrength;

		ImGui::Spacing();

		float brightnessThres = m_BrightnessThreshold;
		if (ImGui::DragFloat("Threshold", &brightnessThres, 0.01f, 0.0f, 10.0f)) m_BrightnessThreshold = brightnessThres;

		float brightnessSoftThres = m_BrightnessSoftThreshold;
		if (ImGui::DragFloat("Soft Threshold", &brightnessSoftThres, 0.01f, 0.0f, 1.0f)) m_BrightnessSoftThreshold = brightnessSoftThres;

		ImGui::Spacing();

		Vector4 tint = m_BloomTint;
		if (ImGui::ColorEdit4("Tint", (float*)&tint)) m_BloomTint = tint;

		float bloomTintStrength = m_BloomTintStrength;
		if (ImGui::DragFloat("Tint Strength", &bloomTintStrength, 0.01f, 0.0f, 100.0f)) m_BloomTintStrength = bloomTintStrength;

		ImGui::Spacing();

		bool enableDirt = m_EnableDirtMask;
		if (ImGui::Checkbox("Dirt Mask", &enableDirt)) m_EnableDirtMask = enableDirt;

		float dirtStrength = m_DirtMaskStrength;
		if (ImGui::DragFloat("Dirt Strength", &dirtStrength, 0.01f, 0.0f, 100.0f)) m_DirtMaskStrength = dirtStrength;

		ImGui::Unindent();
	}
}
