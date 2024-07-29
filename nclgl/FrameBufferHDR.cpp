#include "FrameBufferHDR.h"
#include "common.h"

FrameBufferHDR::~FrameBufferHDR()
{
	glDeleteFramebuffers(1, &m_ProgramID);
	glDeleteRenderbuffers(1, &m_RenderBufferID);
}

void FrameBufferHDR::Bind()
{
	glViewport(0, 0, m_Width, m_Height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ProgramID);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBufferID);
}

void FrameBufferHDR::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferHDR::Invalidate()
{
	m_CaptureProjection = Matrix4::Perspective(1.0f, 10.0f, 1.0f, DegToRad(90.0f));

	m_CaptureViews[0] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::RIGHT, Vector3::DOWN);
	m_CaptureViews[1] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::LEFT, Vector3::DOWN);
	m_CaptureViews[2] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::UP, Vector3::BACK);
	m_CaptureViews[3] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::DOWN, Vector3::FORWARD);
	m_CaptureViews[4] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::BACK, Vector3::DOWN);
	m_CaptureViews[5] = Matrix4::BuildViewMatrix(Vector3::ZERO, Vector3::FORWARD, Vector3::DOWN);

	if (m_ProgramID)
	{
		glDeleteFramebuffers(1, &m_ProgramID);
		glDeleteRenderbuffers(1, &m_RenderBufferID);
	}

	glGenFramebuffers(1, &m_ProgramID);
	glGenRenderbuffers(1, &m_RenderBufferID);

	glBindFramebuffer(GL_FRAMEBUFFER, m_ProgramID);
	glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RenderBufferID);
}