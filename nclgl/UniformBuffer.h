#pragma once

#include <glad/glad.h>

class UniformBuffer
{
public:
	UniformBuffer() = delete;
	UniformBuffer(GLsizeiptr _size, const void* data, GLenum _drawUsage, const unsigned int _bindingIndex, GLsizeiptr _startOffset);
	~UniformBuffer();

	bool IsInitialized();

	void Bind();
	void Unbind();
	void BindData(const unsigned int& index, GLintptr offset, GLsizeiptr size);
	void BindSubData(GLintptr startOffset, GLsizeiptr dataSize, const void* data);

	const unsigned int GetID() const { return m_ProgramID; }
	const unsigned int GetBindingIndex() const { return m_BindingIndex; }
	GLsizeiptr GetBufferSize() const { return m_BufferSize; }
	GLenum GetDrawUsage() const { return m_DrawUsage; }

protected:
	unsigned int m_ProgramID;
	unsigned int m_BindingIndex;
	GLsizeiptr m_BufferSize;
	GLenum m_DrawUsage;
};