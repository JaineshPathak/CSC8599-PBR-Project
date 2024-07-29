#include "PostProcessVignette.h"

PostProcessVignette::PostProcessVignette(const unsigned int& sizeX, const unsigned int& sizeY) :
	m_Amount(15.0f),
	m_FallOff(0.25f),
	PostProcessEffect(sizeX, sizeY)
{
	m_PostEffectType = EPostEffectType::Type_LastPass;

	if (!InitShaders()) return;

	m_VignetteFBO = std::shared_ptr<FrameBuffer>(new FrameBuffer(m_WidthI, m_HeightI, GL_RGB16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, 1, false));
	m_VignetteFBO->RemoveDepthAttachment();
	m_VignetteFBO->SetDeleteDepthTextureStatus(true);

	m_FinalFBO = std::shared_ptr<FrameBuffer>(new FrameBuffer(m_WidthI, m_HeightI, GL_RGB16F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, 1, false));
	m_FinalFBO->RemoveDepthAttachment();
	m_FinalFBO->SetDeleteDepthTextureStatus(true);

	ImGuiRenderer::Get()->AddObserver(m_VignetteFBO);
	ImGuiRenderer::Get()->AddObserver(m_FinalFBO);
	ImGuiRenderer::Get()->RegisterPostProcessItem(this);

	m_IsEnabled = false;
}

PostProcessVignette::~PostProcessVignette()
{
	m_VignetteFBO->Destroy();
	m_FinalFBO->Destroy();
}

bool PostProcessVignette::InitShaders()
{
	m_VignetteShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostBloomVert.glsl", "PostProcess/PostVignetteFrag.glsl"));
	if (!m_VignetteShader->LoadSuccess()) return false;

	m_VignetteFinalShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostBloomVert.glsl", "PostProcess/PostCombinedFinalFrag.glsl"));
	if (!m_VignetteFinalShader->LoadSuccess()) return false;

	return true;
}

const unsigned int PostProcessVignette::GetProcessedTexture() const
{
	return m_FinalFBO->GetColorAttachmentTex();
}

void PostProcessVignette::Render(const unsigned int& sourceTextureID, const unsigned int& depthTextureID)
{
	m_SrcViewportSize = Vector2((float)m_VignetteFBO->GetWidth(), (float)m_VignetteFBO->GetHeight());

	m_VignetteFBO->Bind();
	m_VignetteShader->Bind();

	m_VignetteShader->SetFloat("amount", m_Amount);
	m_VignetteShader->SetFloat("power", m_FallOff);
	m_VignetteShader->SetVector2("srcResolution", m_SrcViewportSize);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	m_QuadMesh->Draw();

	m_VignetteShader->UnBind();
	m_VignetteFBO->Unbind();

	m_FinalFBO->Bind();
	m_VignetteFinalShader->Bind();

	m_VignetteFinalShader->SetTexture("srcTexture", sourceTextureID, 0);
	m_VignetteFinalShader->SetTexture("postTexture", m_VignetteFBO->GetColorAttachmentTex(), 1);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	m_QuadMesh->Draw();

	m_VignetteFinalShader->UnBind();
	m_FinalFBO->Unbind();
}

void PostProcessVignette::OnImGuiRender()
{
	if (ImGui::CollapsingHeader("Vignette"))
	{
		ImGui::Indent();

		bool enable = m_IsEnabled;
		if (ImGui::Checkbox("Enable", &enable)) m_IsEnabled = enable;

		float amt = m_Amount;
		if (ImGui::DragFloat("Amount", &amt, 0.01f, 0.01f, 100.0f)) m_Amount = amt;

		float falloff = m_FallOff;
		if (ImGui::DragFloat("Power", &falloff, 0.01f, 0.01f, 200.0f)) m_FallOff = falloff;

		ImGui::Unindent();
	}
}
