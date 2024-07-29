#pragma once
#include "FrameBuffer.h"
class FrameBufferFP : public FrameBuffer
{
public:
	FrameBufferFP(const unsigned int& sizeX, const unsigned int& sizeY) : FrameBuffer(sizeX, sizeY) { Invalidate(); }

protected:
	virtual void Invalidate() override;
};