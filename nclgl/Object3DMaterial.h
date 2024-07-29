#pragma once
#include "Shader.h"
#include <memory>

class Object3DMaterial
{
public:
	Object3DMaterial() = delete;
	Object3DMaterial(std::shared_ptr<Shader> shaderObject) : m_ShaderObject(shaderObject) {}
	virtual ~Object3DMaterial() {};

	std::shared_ptr<Shader> GetShaderObject() const { return m_ShaderObject; }
	void SetShaderObject(std::shared_ptr<Shader> shaderObject) { m_ShaderObject = shaderObject; }

	virtual void SetInt(const std::string& name, const int& val);
	virtual void SetBool(const std::string& name, const bool& val);
	virtual void SetFloat(const std::string& name, const float& val);
	virtual void SetMat4(const std::string& name, const Matrix4& val, bool transposed = false);
	virtual void SetVector2(const std::string& name, const Vector2& val);
	virtual void SetVector3(const std::string& name, const Vector3& val);
	virtual void SetVector4(const std::string& name, const Vector4& val);
	virtual void SetTexture(const std::string& name, const unsigned int& texID, const int& texSlot);
	virtual void SetTextureCubeMap(const std::string& name, const unsigned int& texID, const int& texSlot);

	virtual void Bind();
	virtual void Unbind();

protected:
	std::shared_ptr<Shader> m_ShaderObject;
};