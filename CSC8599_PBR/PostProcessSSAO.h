#pragma once
#include "PostProcessEffect.h"
#include <random>

class PostProcessSSAO : public PostProcessEffect
{
public:
	PostProcessSSAO(const unsigned int& sizeX, const unsigned int& sizeY);
	~PostProcessSSAO();

private:
	void GenerateKernel();
	void GenerateNoise();

protected:
	virtual bool InitShaders() override;
	virtual bool InitTextures() override;

public:
	virtual const unsigned int GetProcessedTexture() const override;
	virtual void Render(const unsigned int& sourceTextureID, const unsigned int& depthTextureID) override;
	virtual void OnImGuiRender() override;

private:
	std::uniform_real_distribution<float> m_RandomFloats;
	std::default_random_engine m_RandomGenerator;

	int m_KernelSize;
	std::vector<Vector3> m_KernelData;
	std::vector<Vector3> m_NoiseData;

	float m_SampleRadius;
	float m_Intensity;
	Vector2 m_NoiseScale;

	std::shared_ptr<Shader> m_PostSSAOShader;
	std::shared_ptr<Shader> m_PostSSAOBlurShader;
	
	std::shared_ptr<Texture> m_NoiseTexture;
	unsigned int m_NoiseTextureID;
	std::shared_ptr<Texture> m_BlurTexture;
	
	std::shared_ptr<FrameBuffer> m_FinalFBO;
	std::shared_ptr<FrameBuffer> m_BlurFBO;
};