#include "Object3D.h"

#include <thread>
#include <stb_image/stb_image.h>

Object3D::Object3D(const std::string& objectName, const std::string& meshFileName, const std::string& meshMaterialName, const std::string& meshShaderVertexFile, const std::string& meshShaderFragmentFile, const float& lookAtDistance)
{
	m_UseThreads = true;

	m_ObjectName = objectName;
	m_MeshFilename = meshFileName;
	m_MeshMaterialFilename = meshMaterialName;
	m_ShaderVertexFilename = meshShaderVertexFile;
	m_ShaderFragmentFilename = meshShaderFragmentFile;

	m_Position = Vector3::ZERO;
	CalcModelMatrix();

	m_LookAtDistance = lookAtDistance;

	if (!LoadMesh()) { std::cout << "ERROR: 3D Object Failed to load. Something went wrong in loading Mesh!" << std::endl; return; }
	if (!LoadMeshMaterial()) { std::cout << "ERROR: 3D Object Failed to load. Something went wrong in loading Mesh Material!" << std::endl; return; }
	if (!LoadShaders() && m_ShaderVertexFilename.size() > 0 && m_ShaderFragmentFilename.size() > 0) { std::cout << "ERROR: 3D Object Failed to load. Something went wrong in loading Shader!" << std::endl; return; }
	if (!LoadTextures()) { std::cout << "ERROR: 3D Object Failed to load. Something went wrong in loading Textures!" << std::endl; return; }
}

Object3D::~Object3D()
{
	m_TexDiffuseSet.clear();
	m_TexMetallicSet.clear();
	m_TexRoughnessSet.clear();
	m_TexNormalSet.clear();
	m_TexOcclusionSet.clear();
	m_TexEmissionSet.clear();
	m_OpacitySet.clear();
}

bool Object3D::LoadMesh()
{
	m_MeshObject = std::shared_ptr<Mesh>(Mesh::LoadFromMeshFile(m_MeshFilename));
	return m_MeshObject != nullptr;
}

bool Object3D::LoadMeshMaterial()
{
	m_MeshMaterialObject = std::shared_ptr<MeshMaterial>(new MeshMaterial(m_MeshMaterialFilename));
	return m_MeshMaterialObject != nullptr;
}

bool Object3D::LoadShaders()
{
	m_ShaderObject = std::shared_ptr<Shader>(new Shader(m_ShaderVertexFilename, m_ShaderFragmentFilename));
	return m_ShaderObject->LoadSuccess();
}

bool Object3D::LoadTextures()
{
	if (m_MeshObject == nullptr) return false;
	if (m_MeshMaterialObject == nullptr) return false;

	int meshSubCount = m_MeshObject->GetSubMeshCount();
	m_TexDiffuseSet.assign(meshSubCount, -1);
	m_TexMetallicSet.assign(meshSubCount, -1);
	m_TexRoughnessSet.assign(meshSubCount, -1);
	m_TexNormalSet.assign(meshSubCount, -1);
	m_TexOcclusionSet.assign(meshSubCount, -1);
	m_TexEmissionSet.assign(meshSubCount, -1);
	m_OpacitySet.assign(meshSubCount, -1);

	if (m_UseThreads)
	{
		const std::unordered_set<std::string>& texturesList = m_MeshMaterialObject->GetTexturesList();
		for (auto it = texturesList.cbegin(); it != texturesList.cend(); ++it)		
			m_TexturesDataMap[*it] = new TextureData(TEXTUREDIR + *it);

		stbi_set_flip_vertically_on_load(true);
		AddMaterialTextureThreaded();
		stbi_set_flip_vertically_on_load(false);

		for (int i = 0; i < meshSubCount; i++)
		{
			const MeshMaterialEntry* matEntry = m_MeshMaterialObject->GetMaterialForLayer(i);
			if (matEntry != nullptr)
			{
				FillTextureIDsThreaded(i, "Diffuse", matEntry, m_TexDiffuseSet);
				FillTextureIDsThreaded(i, "Metallic", matEntry, m_TexMetallicSet);
				FillTextureIDsThreaded(i, "Roughness", matEntry, m_TexRoughnessSet);
				FillTextureIDsThreaded(i, "Bump", matEntry, m_TexNormalSet);
				FillTextureIDsThreaded(i, "Occlusion", matEntry, m_TexOcclusionSet);
				FillTextureIDsThreaded(i, "Emission", matEntry, m_TexEmissionSet);
				FillTextureIDsThreaded(i, "Opacity", matEntry, m_OpacitySet);
			}
		}
	}
	else
	{
		for (int i = 0; i < meshSubCount; i++)
		{
			const MeshMaterialEntry* matEntry = m_MeshMaterialObject->GetMaterialForLayer(i);
			if (matEntry != nullptr)
			{
				LoadMaterialTextures(i, "Diffuse", matEntry, m_TexDiffuseSet);
				LoadMaterialTextures(i, "Metallic", matEntry, m_TexMetallicSet);
				LoadMaterialTextures(i, "Roughness", matEntry, m_TexRoughnessSet);
				LoadMaterialTextures(i, "Bump", matEntry, m_TexNormalSet);
				LoadMaterialTextures(i, "Occlusion", matEntry, m_TexOcclusionSet);
				LoadMaterialTextures(i, "Emission", matEntry, m_TexEmissionSet);
				LoadMaterialTextures(i, "Opacity", matEntry, m_OpacitySet);
			}
		}
	}	

	return true;
}

#pragma region Non-Multithreading Loading
std::shared_ptr<Texture> Object3D::AddMaterialTexture(const std::string& fileName)
{
	std::unordered_map<std::string, std::shared_ptr<Texture>>::iterator i = m_TexturesSet.find(fileName);
	if (i != m_TexturesSet.end())
		return i->second;

	std::shared_ptr<Texture> tex = std::shared_ptr<Texture>(new Texture(fileName));
	m_TexturesSet.emplace(fileName, tex);

	return tex;
}

bool Object3D::LoadMaterialTextures(const int& index, const std::string& entryName, const MeshMaterialEntry* const materialEntry, std::vector<int>& textureSetContainer)
{
	const std::string* textureFileName = nullptr;
	materialEntry->GetEntry(entryName, &textureFileName);
	if (textureFileName != nullptr)
	{
		std::string filePath = TEXTUREDIR + *textureFileName;
		std::shared_ptr<Texture> tex = AddMaterialTexture(filePath);

		if (tex != nullptr)
			textureSetContainer[index] = tex->GetID();
		else
			return false;
	}
	else
		return false;

	return true;
}
#pragma endregion


#pragma region Multithreading Loading
void Object3D::LoadTextureData(TextureData* texData)
{
	texData->data = stbi_load(texData->path.c_str(), &texData->texWidth, &texData->texHeight, &texData->texChannels, 0);
}

void Object3D::AddMaterialTextureThreaded()
{
	std::vector<std::thread> m_TextureThreads(m_TexturesDataMap.size());
	int i = 0;
	for (auto it = m_TexturesDataMap.begin(); it != m_TexturesDataMap.end(); ++it)
	{
		m_TextureThreads[i] = std::thread(&Object3D::LoadTextureData, this, it->second);
		i++;
	}

	for (auto& t : m_TextureThreads)
		t.join();

	m_TextureThreads.clear();

	for (auto it = m_TexturesDataMap.begin(); it != m_TexturesDataMap.end(); ++it)
	{
		const TextureData& texData = *it->second;
		std::shared_ptr<Texture> tex = std::shared_ptr<Texture>(new Texture(texData));
		m_TexturesSet.emplace((*it).first, tex);

		delete it->second;
	}

	m_TexturesDataMap.clear();
}

bool Object3D::LoadMaterialTexturesThreaded(const std::string& entryName, const MeshMaterialEntry* const materialEntry)
{
	const std::string* textureFileName = nullptr;
	materialEntry->GetEntry(entryName, &textureFileName);
	if (textureFileName != nullptr)
	{
		std::string filePath = TEXTUREDIR + *textureFileName;
		m_TexturesDataMap[*textureFileName] = new TextureData(filePath);
	}
	else
		return false;

	return true;
}

void Object3D::FillTextureIDsThreaded(const int& index, const std::string& entryName, const MeshMaterialEntry* const materialEntry, std::vector<int>& textureSetContainer)
{
	const std::string* textureFileName = nullptr;
	materialEntry->GetEntry(entryName, &textureFileName);
	if (textureFileName != nullptr)
	{
		std::string filePath = TEXTUREDIR + *textureFileName;
		textureSetContainer[index] = m_TexturesSet[*textureFileName]->GetID();
	}
}
#pragma endregion


void Object3D::CalcModelMatrix()
{
	m_ModelMatrix = Matrix4::Translation(m_Position) * Matrix4::Scale(Vector3(1.0f, 1.0f, 1.0f));
}

void Object3D::Draw()
{
	if (m_MeshObject == nullptr) return;

	for (int i = 0; i < m_MeshObject->GetSubMeshCount(); i++)
		m_MeshObject->DrawSubMesh(i);
}

void Object3D::Draw(const int& index)
{
	if (m_MeshObject == nullptr) return;

	m_MeshObject->DrawSubMesh(index);
}
