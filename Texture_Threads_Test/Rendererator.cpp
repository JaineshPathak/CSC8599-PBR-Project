#include "Rendererator.h"
#include <chrono>
#include <stb_image/stb_image.h>

int index = 0;
bool useThreads = true;
Rendererator::Rendererator(Window& parent) : OGLRenderer(parent)
{
	quadMesh = Mesh::GenerateQuad();
	diffuseShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");

	auto start = std::chrono::high_resolution_clock::now();

	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Helmet/Helmet_BaseColor_sRGB.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Helmet/Helmet_Metallic_Raw.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Helmet/Helmet_Roughness_Raw.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Helmet/Helmet_Normal_Raw.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Helmet/Helmet_Emissive_sRGB.png"));

	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Character/CPP_basecolor.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Character/CPP_metallic.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Character/CPP_roughness.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Character/CPP_normal.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Character/CPP_AO.png"));

	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Character/CPD_basecolor.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Character/CPD_metallic.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Character/CPD_roughness.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Character/CPD_normal.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Character/CPD_AO.png"));	

	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BODY_ALD.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BODY_METALNESS.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BODY_ROUGHNESS.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BODY_NRM.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BODY_AO.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BODY_EMS.png"));

	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/RIM_ALD.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/RIM_METALLIC.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/RIM_ROUGHNESS.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/RIM_NRM.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/RIM_AO.png"));

	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/WHEEL_ALD.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/WHEEL_METALLIC.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/WHEEL_ROUGHNESS.png"));

	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BRAKE_ALD.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BRAKE_METALLIC.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BRAKE_ROUGHNESS.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BRAKE_NRM.png"));
	m_TextureDatas.emplace_back(TextureData(TEXTUREDIR"Car/BRAKE_AO.png"));
	
	m_Textures.reserve(m_TextureDatas.size());
	m_Textures.assign(m_TextureDatas.size(), nullptr);
	if (useThreads)
	{
		stbi_set_flip_vertically_on_load(true);
		std::vector<std::thread> m_TextureThreads(m_TextureDatas.size());
		for (int i = 0; i < m_TextureDatas.size(); i++)
			m_TextureThreads[i] = std::thread(&Rendererator::LoadTextureData, this, std::ref(m_TextureDatas[i]));

		for (auto& t : m_TextureThreads)
			t.join();

		for (int i = 0; i < m_TextureDatas.size(); i++)
			m_Textures[i] = new Texture(m_TextureDatas[i]);

		stbi_set_flip_vertically_on_load(false);
	}
	else
	{
		for (int i = 0; i < m_TextureDatas.size(); i++)
			m_Textures[i] = new Texture(m_TextureDatas[i].path);
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
	std::cout << "Textures Load Time: " << duration.count() << "s" << std::endl;

	init = true;
}

void Rendererator::LoadTextureData(TextureData& texData)
{
	texData.data = stbi_load(texData.path.c_str(), &texData.texWidth, &texData.texHeight, &texData.texChannels, 0);
}

void Rendererator::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	BindShader(diffuseShader);
	UpdateShaderMatrices();
	diffuseShader->SetTexture("diffuseTex", m_Textures[index]->GetID(), 0);
	quadMesh->Draw();
}

void Rendererator::UpdateScene(float dt)
{
	if (Window::GetKeyboard()->KeyTriggered(KeyboardKeys::KEYBOARD_LEFT))
	{
		index = (index - 1) % m_Textures.size();
	}
	if (Window::GetKeyboard()->KeyTriggered(KeyboardKeys::KEYBOARD_RIGHT))
	{
		index = (index + 1) % m_Textures.size();
	}
}