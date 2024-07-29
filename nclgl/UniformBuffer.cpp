#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(GLsizeiptr _size, const void* data, GLenum _drawUsage, const unsigned int _bindingIndex, GLsizeiptr _startOffset)
{
	m_BufferSize = _size;
	m_BindingIndex = _bindingIndex;
	m_DrawUsage = _drawUsage;

	glGenBuffers(1, &m_ProgramID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_ProgramID);
	glBufferData(GL_UNIFORM_BUFFER, m_BufferSize, data, m_DrawUsage);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, m_BindingIndex, m_ProgramID, _startOffset, m_BufferSize);
}

UniformBuffer::~UniformBuffer()
{
	glDeleteBuffers(1, &m_ProgramID);
}

bool UniformBuffer::IsInitialized()
{
	return glIsBuffer(m_ProgramID);
}

void UniformBuffer::Bind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_ProgramID);
}

void UniformBuffer::Unbind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::BindData(const unsigned int& index, GLintptr offset, GLsizeiptr size)
{
	glBindBufferRange(GL_UNIFORM_BUFFER, index, m_ProgramID, offset, size);
}

void UniformBuffer::BindSubData(GLintptr startOffset, GLsizeiptr dataSize, const void* data)
{
	glBufferSubData(GL_UNIFORM_BUFFER, startOffset, dataSize, data);
}
