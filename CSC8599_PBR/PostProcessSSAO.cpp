#include "PostProcessSSAO.h"

PostProcessSSAO::PostProcessSSAO(const unsigned int& sizeX, const unsigned int& sizeY) :
    m_KernelSize(64),
    m_SampleRadius(0.5f),
    m_Intensity(2.0),
    m_NoiseScale(Vector2((float)sizeX / 4.0f, (float)sizeY / 4.0f)),
    PostProcessEffect(sizeX, sizeY)
{
    m_PostEffectType = EPostEffectType::Type_PrePass;

    m_RandomFloats = std::uniform_real_distribution<float>(0.0f, 1.0f);
    GenerateKernel();
    GenerateNoise();

    if (!InitShaders()) return;
    if (!InitTextures()) return;

    m_FinalFBO = std::shared_ptr<FrameBuffer>(new FrameBuffer(m_WidthI, m_HeightI, GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, 1, false));
    m_FinalFBO->RemoveDepthAttachment();
    m_FinalFBO->SetDeleteDepthTextureStatus(true);

    m_BlurFBO = std::shared_ptr<FrameBuffer>(new FrameBuffer(m_WidthI, m_HeightI, GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, 1, false));
    m_BlurFBO->RemoveDepthAttachment();
    m_BlurFBO->SetDeleteDepthTextureStatus(true);

    ImGuiRenderer::Get()->AddObserver(m_FinalFBO);
    ImGuiRenderer::Get()->AddObserver(m_BlurFBO);
    ImGuiRenderer::Get()->RegisterPostProcessItem(this);

    m_IsEnabled = true;
}

PostProcessSSAO::~PostProcessSSAO()
{
    m_FinalFBO->Destroy();
    m_BlurFBO->Destroy();
}

void PostProcessSSAO::GenerateKernel()
{
    m_KernelData.reserve(m_KernelSize);
    for (int i = 0; i < m_KernelSize; i++)
    {
        Vector3 sample(m_RandomFloats(m_RandomGenerator) * 2.0f - 1.0f, m_RandomFloats(m_RandomGenerator) * 2.0f - 1.0f, m_RandomFloats(m_RandomGenerator));
        sample.Normalise();

        float rnd = m_RandomFloats(m_RandomGenerator);
        sample.x *= rnd;
        sample.y *= rnd;
        sample.z *= rnd;

        float scale = (float)i / (float)m_KernelSize;
        scale = naive_lerp(0.1f, 1.0f, scale * scale);
        sample.x *= scale;
        sample.y *= scale;
        sample.z *= scale;

        m_KernelData.emplace_back(sample);
    }
}

void PostProcessSSAO::GenerateNoise()
{    
    m_NoiseData.reserve(16);
    for (int i = 0; i < 16; i++)
    {
        Vector3 noise(m_RandomFloats(m_RandomGenerator) * 2.0f - 1.0f, m_RandomFloats(m_RandomGenerator) * 2.0f - 1.0f, 0.0f);
        m_NoiseData.emplace_back(noise);
    }    
}

bool PostProcessSSAO::InitShaders()
{
    m_PostSSAOShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostSSAOVert.glsl", "PostProcess/PostSSAOFrag.glsl"));
    if (!m_PostSSAOShader->LoadSuccess()) return false;

    m_PostSSAOBlurShader = std::shared_ptr<Shader>(new Shader("PostProcess/PostSSAOBlurVert.glsl", "PostProcess/PostSSAOBlurFrag.glsl"));
    if (!m_PostSSAOBlurShader->LoadSuccess()) return false;

    return true;
}

bool PostProcessSSAO::InitTextures()
{
    m_NoiseTexture = std::shared_ptr<Texture>(new Texture(4, 4, GL_RGBA16F, GL_RGB, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_REPEAT, false));
    if (!m_NoiseTexture->IsInitialized()) return false;
    m_NoiseTexture->UploadData(m_NoiseData.data());

    m_BlurTexture = std::shared_ptr<Texture>(new Texture(m_WidthI, m_HeightI, GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, false));
    if (!m_BlurTexture->IsInitialized()) return false;

    return true;
}

const unsigned int PostProcessSSAO::GetProcessedTexture() const
{
    return m_BlurFBO->GetColorAttachmentTex();
}

void PostProcessSSAO::Render(const unsigned int& sourceTextureID, const unsigned int& depthTextureID)
{
    m_FinalFBO->Bind();
    m_PostSSAOShader->Bind();
    m_PostSSAOShader->SetTexture("depthTex", depthTextureID, 0);
    //m_PostSSAOShader->SetTexture("positionTex", Renderer::Get()->GetPositionFrameBuffer()->GetColorAttachmentTex(), 1);
    //m_PostSSAOShader->SetTexture("normalTex", Renderer::Get()->GetNormalsFrameBuffer()->GetColorAttachmentTex(), 2);
    m_PostSSAOShader->SetTexture("noiseTex", m_NoiseTexture->GetID(), 1);
    
    m_PostSSAOShader->SetFloat("sampleRadius", m_SampleRadius);
    m_PostSSAOShader->SetVector2("noiseScale", m_NoiseScale);
    m_PostSSAOShader->SetFloat("intensity", m_Intensity);
    
    for (int i = 0; i < m_KernelSize; i++)
        m_PostSSAOShader->SetVector3("kernel[" + std::to_string(i) + "]", m_KernelData[i]);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_QuadMesh->Draw();

    m_PostSSAOShader->UnBind();
    m_FinalFBO->Unbind();

    
    m_BlurFBO->Bind();
    m_PostSSAOBlurShader->Bind();
    m_PostSSAOBlurShader->SetTexture("ssaoTexture", m_FinalFBO->GetColorAttachmentTex(), 0);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_QuadMesh->Draw();

    m_PostSSAOBlurShader->UnBind();
    m_BlurFBO->Unbind();
}

void PostProcessSSAO::OnImGuiRender()
{
    if (ImGui::CollapsingHeader("SSAO"))
    {
        ImGui::Indent();

        bool enable = m_IsEnabled;
        if (ImGui::Checkbox("Enable", &enable)) m_IsEnabled = enable;

        float rad = m_SampleRadius;
        if (ImGui::DragFloat("Radius", &rad, 0.01f, 0.1f, 10.0f)) m_SampleRadius = rad;

        float intensity = m_Intensity;
        if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 1.0f, 20.0f)) m_Intensity = intensity;

        ImGui::Unindent();
    }
}