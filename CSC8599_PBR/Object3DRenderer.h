#pragma once
#include "IImguiItem.h"
#include "ImGuiRenderer.h"

#include <memory>
#include <unordered_map>

class Object3DEntity;
class Shader;
class LookAtCamera;
class Texture;
class FrameBuffer;
class Object3DRenderer : public IImguiItem
{
public:
	Object3DRenderer(const float& width, const float& height);
	~Object3DRenderer();

protected:
	bool InitBuffers();
	bool InitShaders();
	bool InitMeshes();

public:
	const bool IsInitialized() const { return m_IsInitialized; }
	const unsigned int GetDepthTexture() const;
	const unsigned int GetShadowDepthTexture() const;
	const Matrix4 GetLightSpaceMatrix() const { return m_LightSpaceMatrix; }

private:
	std::shared_ptr<Object3DEntity> Add3DObject(const std::string& objectName, const std::string& objectMeshFile, const std::string& objectMeshMaterialFile, const float& lookAtDistance = 3.0f);
	void ChangeShaderMode(const int& newShaderMode);
	void OnObject3DChanged();

	void RenderUsualDepths();
	void RenderShadowDepths();

public:
	void Draw();
	void RenderDepths();
	void Render();
	void RenderPlatform();
	virtual void OnImGuiRender() override;

protected:
	Matrix4 m_LightSpaceMatrix;

private:
	float m_Width, m_Height;
	int m_ShaderMode;

	std::shared_ptr<LookAtCamera> m_MainCamera;
	std::shared_ptr<Shader> m_PBRShader;
	std::shared_ptr<Shader> m_BlinnShader;
	std::shared_ptr<Shader> m_DisneyShader;
	std::shared_ptr<Shader> m_OrenNayarShader;

	std::shared_ptr<Shader> m_DepthBufferShader;
	std::shared_ptr<Shader> m_ShadowDepthBufferShader;	

	std::shared_ptr<FrameBuffer> m_DepthFrameBuffer;
	std::shared_ptr<FrameBuffer> m_ShadowDepthFrameBuffer;

	std::shared_ptr<Texture> m_ShaderTex;

	static unsigned int s_3DEntityIDs;
	char** m_3DEntitiesNamesList;
	std::vector<std::string> m_3DEntitiesNames;
	int m_Current3DEntityIndex;
	std::unordered_map<unsigned int, std::shared_ptr<Object3DEntity>> m_3DEntities;

	std::shared_ptr<Object3DEntity> m_3DPlatform;

	bool m_IsInitialized;
};