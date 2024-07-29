#pragma once
#include "FrameBuffer.h"
#include "Vector2.h"
#include <vector>
#include <memory>

class Texture;

struct BloomMip
{
	Vector2 size;
	int sizeX, sizeY;
	unsigned int texture;
};

class FrameBufferBloom : public FrameBuffer
{
public:
	FrameBufferBloom();
	FrameBufferBloom(const unsigned int& sizeX, const unsigned int& sizeY, const int& numMipsChain);
	~FrameBufferBloom();

	virtual void Destroy() override;
	virtual void Invalidate() override;

	void Recalculate(const int& numMipsChain);

	const std::vector<BloomMip>& MipChain() const { return m_MipChain; }

private:
	std::vector<BloomMip> m_MipChain;
};