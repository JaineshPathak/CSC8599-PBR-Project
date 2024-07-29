#pragma once
#include <memory>
#include <vector>
#include <unordered_map>

#include "Mesh.h"
#include "MeshMaterial.h"
#include "Texture.h"
#include "Shader.h"
#include "Matrix4.h"

class Object3D
{
public:
	Object3D(const std::string& objectName, const std::string& meshFileName, const std::string& meshMaterialName, const std::string& meshShaderVertexFile = "", const std::string& meshShaderFragmentFile = "", const float& lookAtDistance = 3.0f);
	virtual ~Object3D();

protected:
	virtual bool LoadMesh() final;
	virtual bool LoadMeshMaterial() final;
	virtual bool LoadShaders() final;
	virtual bool LoadTextures() final;

	virtual std::shared_ptr<Texture> AddMaterialTexture(const std::string& fileName) final;
	virtual bool LoadMaterialTextures(const int& index, const std::string& entryName, const MeshMaterialEntry* const materialEntry, std::vector<int>& textureSetContainer) final;

	//Threads Part
	virtual void LoadTextureData(TextureData* texData) final;
	virtual void AddMaterialTextureThreaded() final;
	virtual bool LoadMaterialTexturesThreaded(const std::string& entryName, const MeshMaterialEntry* const materialEntry) final;
	virtual void FillTextureIDsThreaded(const int& index, const std::string& entryName, const MeshMaterialEntry* const materialEntry, std::vector<int>& textureSetContainer);

public:
	const std::string GetObjectName() const { return m_ObjectName; }
	std::shared_ptr<Mesh> GetObjectMesh() const { return m_MeshObject; }
	std::shared_ptr<MeshMaterial> GetObjectMeshMaterial() const { return m_MeshMaterialObject; }

	std::shared_ptr<Shader> GetObjectShader() const { return m_ShaderObject; }
	void SetObjectShader(std::shared_ptr<Shader> newShaderObject) { m_ShaderObject = newShaderObject; }

	Matrix4 GetModelMatrix() const { return m_ModelMatrix; }
	void CalcModelMatrix();

	Vector3 GetPosition() const { return m_Position; }
	void SetPosition(const Vector3& newPosition) { m_Position = newPosition; CalcModelMatrix(); }

	const float GetLookDistance() const { return m_LookAtDistance; }
	virtual void SetLookDistance(const float& newDistance) { m_LookAtDistance; }

	virtual void Draw();
	virtual void Draw(const int& index);
	virtual void Render() = 0;

protected:
	std::string m_ObjectName;
	std::string m_MeshFilename;
	std::string m_MeshMaterialFilename;
	std::string m_ShaderVertexFilename;
	std::string m_ShaderFragmentFilename;

	std::shared_ptr<Mesh> m_MeshObject;
	std::shared_ptr<MeshMaterial> m_MeshMaterialObject;
	std::shared_ptr<Shader> m_ShaderObject;

	std::unordered_map<std::string, std::shared_ptr<Texture>> m_TexturesSet;
	std::vector<int> m_TexDiffuseSet;
	std::vector<int> m_TexMetallicSet;
	std::vector<int> m_TexRoughnessSet;
	std::vector<int> m_TexNormalSet;
	std::vector<int> m_TexOcclusionSet;
	std::vector<int> m_TexEmissionSet;
	std::vector<int> m_OpacitySet;

	Vector3 m_Position;
	Matrix4 m_ModelMatrix;

	float m_LookAtDistance;

	//Threads Part
	bool m_UseThreads;
	std::unordered_map<std::string, TextureData*> m_TexturesDataMap;
};