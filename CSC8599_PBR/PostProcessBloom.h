#pragma once
#include "PostProcessEffect.h"
#include <nclgl/FrameBufferBloom.h>

class PostProcessBloom : public PostProcessEffect
{
public:
	PostProcessBloom(const unsigned int& sizeX, const unsigned int& sizeY);
	~PostProcessBloom();

protected:
	virtual bool InitShaders() override;
	virtual bool InitTextures() override;

private:
	void RenderBrightColors(unsigned int srcTexture);
	void RenderDownSamples(unsigned int srcTexture);
	void RenderUpSamples();
	void RenderBloomTexture(unsigned int srcTexture);
	void RenderBloomFinal();
	void RenderBloomCombined();

	void OnIterationsChanged();

	unsigned int GetBloomTexture(int index = 0);

public:
	virtual const unsigned int GetProcessedTexture() const override;
	virtual void Render(const unsigned int& sourceTextureID, const unsigned int& depthTextureID) override;
	virtual void OnImGuiRender() override;

private:
	int m_BloomIterations;
	bool m_EnableBloomDebug;
	float m_BloomFilterRadius;

	float m_BrightnessThreshold;
	float m_BrightnessSoftThreshold;

	float m_BloomStrength;

	Vector4 m_BloomTint;
	float m_BloomTintStrength;

	bool m_EnableDirtMask;
	float m_DirtMaskStrength;

	unsigned int m_SourceTextureID;

	std::shared_ptr<Texture> m_DirtMaskTexture;
	std::shared_ptr<Shader> m_PostBloomBrightenShader;
	std::shared_ptr<Shader> m_PostBloomDownSampleShader;
	std::shared_ptr<Shader> m_PostBloomUpSampleShader;
	std::shared_ptr<Shader> m_PostBloomFinalShader;
	std::shared_ptr<Shader> m_PostBloomCombinedShader;

	std::shared_ptr<FrameBufferBloom> m_BloomFBO;
	std::shared_ptr<FrameBuffer> m_BrightenFBO;
	std::shared_ptr<FrameBuffer> m_CombinedFBO;
	std::shared_ptr<FrameBuffer> m_FinalFBO;
};