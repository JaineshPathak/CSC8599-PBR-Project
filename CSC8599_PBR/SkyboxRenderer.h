#pragma once
#include "IImguiItem.h"
#include "ImGuiRenderer.h"
#include <vector>

class UniformBuffer;
class Shader;
class Matrix4;
class Texture;
class TextureHDR;
class TextureEnvCubeMap;
class TextureCubeMap;
class FrameBufferFP;
class FrameBufferHDR;
class UniformBuffer;

struct SkyboxData
{
	Vector4 data;
};

struct SkyboxCubeMap
{
	SkyboxCubeMap(const std::string& fileName, const std::string& skyboxName);

	std::string m_SkyboxFileName;
	std::string m_SkyboxName;
	std::string m_SkyboxFileNameNoExt;

	bool m_AlreadyCapturedCubeMap;
	bool m_AlreadyCapturedIrradianceMap;
	bool m_AlreadyCapturedPreFilterMipMaps;
	bool m_AlreadyCapturedBRDFLUTMap;

	std::shared_ptr<TextureHDR> m_CubeMapHDRTexture;
	std::shared_ptr<TextureEnvCubeMap> m_CubeMapEnvTexture;
	std::shared_ptr<TextureEnvCubeMap> m_CubeMapIrradianceTexture;
	std::shared_ptr<TextureEnvCubeMap> m_CubeMapPreFilterTexture;
	std::shared_ptr<Texture> m_BRDFLUTTexture;
};

class SkyboxRenderer : public IImguiItem
{
public:
	SkyboxRenderer();
	~SkyboxRenderer();

	const bool IsInitialized() const { return m_IsInitialized; }

	std::shared_ptr<TextureEnvCubeMap> GetEnvCubeMapTexture() { return m_SkyboxesList[m_SkyboxesIndexCurrent].m_CubeMapEnvTexture; }
	std::shared_ptr<TextureEnvCubeMap> GetIrradianceTexture() { return m_SkyboxesList[m_SkyboxesIndexCurrent].m_CubeMapIrradianceTexture; }
	std::shared_ptr<TextureEnvCubeMap> GetPreFilterTexture() { return m_SkyboxesList[m_SkyboxesIndexCurrent].m_CubeMapPreFilterTexture; }
	std::shared_ptr<Texture> GetBRDFLUTTexture() { return m_SkyboxesList[m_SkyboxesIndexCurrent].m_BRDFLUTTexture; }

protected:
	bool InitShaders();
	bool InitBuffers();
	bool InitTextures();

	void AddSkyboxCubeMap(const std::string& fileName, const std::string& skyboxName);

	void CaptureHDRCubeMap();
	void CaptureIrradianceMap();
	void CapturePreFilterMipMaps();
	void CaptureBRDFLUTMap();

	void RenderSkybox();
	
	void BindSkyboxUBOData();
	void CheckAllSkyboxCaptures();
	void OnSkyboxDataChanged();

public:
	void Render();	
	virtual void OnImGuiRender() override;

protected:
	bool m_IsInitialized;	

	float m_Exposure;
	float m_Gamma;

	Matrix4 m_CaptureProjection;
	Matrix4 m_CaptureViews[6];

	std::shared_ptr<Shader> m_CubeMapShader;
	std::shared_ptr<Shader> m_EquiRect2CubeMapShader;
	std::shared_ptr<Shader> m_IrradianceCubeMapShader;
	std::shared_ptr<Shader> m_PreFilterCubeMapShader;
	std::shared_ptr<Shader> m_BRDFIntegrateShader;

	std::shared_ptr<FrameBufferFP> m_CaptureHDRFrameBuffer;
	std::shared_ptr<FrameBufferFP> m_CaptureIrradianceFrameBuffer;
	std::shared_ptr<FrameBufferHDR> m_CapturePreFilterFrameBuffer;

	SkyboxData m_SkyboxData;
	std::shared_ptr<UniformBuffer> m_SkyboxUBO;
	bool m_CheckedAllSkyboxes;

	char** m_SkyboxesNamesList;
	std::vector<std::string> m_SkyBoxesNames;
	int m_SkyboxesIndexCurrent;
	std::vector<SkyboxCubeMap> m_SkyboxesList;
};