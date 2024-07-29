#pragma once
#include "FrameBuffer.h"
#include "Matrix4.h"

class FrameBufferHDR : public FrameBuffer
{
public:
	FrameBufferHDR(const unsigned int& sizeX, const unsigned int& sizeY) : m_RenderBufferID(-1), FrameBuffer(sizeX, sizeY) 
	{
		Invalidate();
	}
	~FrameBufferHDR();

	const unsigned int GetRenderBufferID() const { return m_RenderBufferID; }
	const Matrix4 GetCaptureProjection() const { return m_CaptureProjection; }
	const Matrix4* GetCaptureViews() const { return m_CaptureViews; }

	virtual void Bind() override;
	virtual void Unbind() override;

protected:
	virtual void Invalidate() override;

	unsigned int m_RenderBufferID;
	Matrix4 m_CaptureProjection;
	Matrix4 m_CaptureViews[6];
};