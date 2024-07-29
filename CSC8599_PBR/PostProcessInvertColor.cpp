#include "PostProcessInvertColor.h"

PostProcessInvertColor::PostProcessInvertColor(const unsigned int& sizeX, const unsigned int& sizeY) : 
    PostProcessEffect(sizeX, sizeY)
{
    m_PostEffectType = EPostEffectType::Type_LastPass;

    if (!InitShaders()) return;

    m_FinalFBO = std::shared_ptr<FrameBuffer>(new FrameBuffer(m_WidthI, m_HeightI, GL_RGB16F, GL_RGB, GL_FLOAT, 1));
    m_FinalFBO->RemoveDepthAttachment();
    m_FinalFBO->SetDeleteDepthTextureStatus(true);

    ImGuiRenderer::Get()->AddObserver(m_FinalFBO);
    ImGuiRenderer::Get()->RegisterPostProcessItem(this);

    m_IsEnabled = false;
}

PostProcessInvertColor::~PostProcessInvertColor()
{
    m_FinalFBO->Destroy();
}

bool PostProcessInvertColor::InitShaders()
{
    m_PostInvertShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostBloomVert.glsl", "PostProcess/PostInvertFrag.glsl"));
    if (!m_PostInvertShader->LoadSuccess()) return false;

    return true;
}

const unsigned int PostProcessInvertColor::GetProcessedTexture() const
{
    return m_FinalFBO->GetColorAttachmentTex();
}

void PostProcessInvertColor::Render(const unsigned int& sourceTextureID, const unsigned int& depthTextureID)
{
    m_FinalFBO->Bind();
    m_PostInvertShader->Bind();
    m_PostInvertShader->SetTexture("srcTexture", sourceTextureID, 0);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_QuadMesh->Draw();

    m_PostInvertShader->UnBind();
    m_FinalFBO->Unbind();
}

void PostProcessInvertColor::OnImGuiRender()
{
    if (ImGui::CollapsingHeader("Invert Color"))
    {
        ImGui::Indent();

        bool enable = m_IsEnabled;
        if (ImGui::Checkbox("Enable", &enable)) m_IsEnabled = enable;

        ImGui::Unindent();
    }
}