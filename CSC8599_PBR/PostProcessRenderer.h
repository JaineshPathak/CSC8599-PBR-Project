#pragma once
#include "IImguiItem.h"
#include "ImGuiRenderer.h"

#include "PostProcessBloom.h"
#include "PostProcessInvertColor.h"
#include "PostProcessSSAO.h"
#include "PostProcessVignette.h"

#include <vector>

class PostProcessRenderer
{
public:
	PostProcessRenderer() = delete;
	PostProcessRenderer(const unsigned int& sizeX, const unsigned int& sizeY);
	~PostProcessRenderer();

	const unsigned int GetMidFinalTexture() const;
	const bool IsInitialized() const { return m_IsInitialized; }

	//SSAO Flags
	const bool IsSSAOEnabled() const { return m_PrePassPostEffects[0]->IsEnabled(); }
	void RenderSSAOPass(const unsigned int& depthTextureID);
	const unsigned int GetSSAOProcessedTexture() const { return m_PrePassPostEffects[0]->GetProcessedTexture(); }

	const bool IsEnabled() const { return m_IsEnabled; }
	void SetActive(const bool& status) { m_IsEnabled = status; }

	const unsigned int GetPreFinalTextureID() const { return m_PreFinalTextureID; }
	const unsigned int GetMidFinalTextureID() const { return m_FinalTextureID; }
	const unsigned int GetLastFinalTextureID() const { return m_LastFinalTextureID; }

	void RenderPrePass(const unsigned int& srcTexture, const unsigned int& depthTextureID);
	void Render(const unsigned int& srcTexture, const unsigned int& depthTextureID);
	void RenderLastPass(const unsigned int& srcTexture, const unsigned int& depthTextureID);

private:
	bool GetEnableStatus();

	void FillPreActivePostEffects();
	void RenderPreActivePostEffects(const unsigned int& srcTexture, const unsigned int& depthTextureID);

	void FillMidActivePostEffects();
	void RenderMidActivePostEffects(const unsigned int& srcTexture, const unsigned int& depthTextureID);
	void RenderMidFinalPostEffect();

	void FillLastActivePostEffects();
	void RenderLastActivePostEffects(const unsigned int& srcTexture, const unsigned int& depthTextureID);

	void AddPostProcessEffect(std::shared_ptr<PostProcessEffect> effect);

protected:
	bool m_IsEnabled;
	bool m_IsInitialized;

	std::shared_ptr<FrameBuffer> m_FinalFBO;
	std::shared_ptr<Shader> m_PostFinalShader;

private:
	unsigned int m_PreFinalTextureID;
	unsigned int m_FinalTextureID;
	unsigned int m_LastFinalTextureID;

	std::vector<std::shared_ptr<PostProcessEffect>> m_PrePassPostEffects;
	std::vector<std::shared_ptr<PostProcessEffect>> m_ActivePrePassPostEffects;

	std::vector<std::shared_ptr<PostProcessEffect>> m_MidPostEffects;
	std::vector<std::shared_ptr<PostProcessEffect>> m_ActiveMidPostEffects;

	std::vector<std::shared_ptr<PostProcessEffect>> m_LastPassPostEffects;
	std::vector<std::shared_ptr<PostProcessEffect>> m_ActiveLastPassPostEffects;

	std::vector<std::shared_ptr<PostProcessEffect>> m_AllPostEffects;

	unsigned int m_Width, m_Height;
};