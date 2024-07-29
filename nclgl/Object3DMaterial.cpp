#include "Object3DMaterial.h"

void Object3DMaterial::SetInt(const std::string& name, const int& val)
{
	if (m_ShaderObject == nullptr) return;
	m_ShaderObject->SetInt(name, val);
}

void Object3DMaterial::SetBool(const std::string& name, const bool& val)
{
	if (m_ShaderObject == nullptr) return;
	m_ShaderObject->SetBool(name, val);
}

void Object3DMaterial::SetFloat(const std::string& name, const float& val)
{
	if (m_ShaderObject == nullptr) return;
	m_ShaderObject->SetFloat(name, val);
}

void Object3DMaterial::SetMat4(const std::string& name, const Matrix4& val, bool transposed)
{
	if (m_ShaderObject == nullptr) return;
	m_ShaderObject->SetMat4(name, val, transposed);
}

void Object3DMaterial::SetVector2(const std::string& name, const Vector2& val)
{
	if (m_ShaderObject == nullptr) return;
	m_ShaderObject->SetVector2(name, val);
}

void Object3DMaterial::SetVector3(const std::string& name, const Vector3& val)
{
	if (m_ShaderObject == nullptr) return;
	m_ShaderObject->SetVector3(name, val);
}

void Object3DMaterial::SetVector4(const std::string& name, const Vector4& val)
{
	if (m_ShaderObject == nullptr) return;
	m_ShaderObject->SetVector4(name, val);
}

void Object3DMaterial::SetTexture(const std::string& name, const unsigned int& texID, const int& texSlot)
{
	if (m_ShaderObject == nullptr) return;
	m_ShaderObject->SetTexture(name, texID, texSlot);
}

void Object3DMaterial::SetTextureCubeMap(const std::string& name, const unsigned int& texID, const int& texSlot)
{
	if (m_ShaderObject == nullptr) return;
	m_ShaderObject->SetTextureCubeMap(name, texID, texSlot);
}

void Object3DMaterial::Bind()
{
	if (m_ShaderObject == nullptr)
	{
		std::cout << "ERROR: Object 3D Material Shader is NULL! Unable to Bind!" << std::endl;
		return;
	}

	m_ShaderObject->Bind();
}

void Object3DMaterial::Unbind()
{
	if (m_ShaderObject == nullptr)
	{
		std::cout << "ERROR: Object 3D Material Shader is NULL! Unable to Unbind!" << std::endl;
		return;
	}

	m_ShaderObject->UnBind();
}