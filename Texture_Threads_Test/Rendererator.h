#pragma once
#include <nclgl/OGLRenderer.h>
#include <nclgl/Texture.h>
#include <memory>
#include <thread>

class Rendererator : public OGLRenderer
{
public:
	Rendererator(Window& parent);

	void LoadTextureData(TextureData& texData);

public:
	void RenderScene() override;
	void UpdateScene(float dt) override;

private:
	Shader* diffuseShader;
	Mesh* quadMesh;

	std::vector<TextureData> m_TextureDatas;
	std::vector<Texture*> m_Textures;
};