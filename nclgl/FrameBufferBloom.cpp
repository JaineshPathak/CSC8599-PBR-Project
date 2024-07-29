#include "FrameBufferBloom.h"
#include "Texture.h"

FrameBufferBloom::FrameBufferBloom() : FrameBuffer(0, 0, 0)
{
}

FrameBufferBloom::FrameBufferBloom(const unsigned int& sizeX, const unsigned int& sizeY, const int& numMipsChain) : FrameBuffer(sizeX, sizeY, numMipsChain)
{
	Invalidate();
}

FrameBufferBloom::~FrameBufferBloom()
{
	Destroy();
}

void FrameBufferBloom::Destroy()
{
	if ((int)m_MipChain.size() > 0)
	{
		for (int i = 0; i < (int)m_MipChain.size(); i++)
		{
			glDeleteTextures(1, &m_MipChain[i].texture);
			m_MipChain[i].texture = 0;
		}
	}
	m_MipChain.clear();

	glDeleteFramebuffers(1, &m_ProgramID);
	m_ProgramID = 0;
}

void FrameBufferBloom::Invalidate()
{
	if (m_ProgramID > 0)
		Destroy();

	glCreateFramebuffers(1, &m_ProgramID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ProgramID);

	Vector2 mipSize = Vector2((float)m_Width, (float)m_Height);
	int mipSizeX = m_Width, mipSizeY = m_Height;

	for (int i = 0; i < m_NumColorAttachments; i++)
	{
		mipSize.x *= 0.5f;
		mipSize.y *= 0.5f;

		mipSizeX /= 2;
		mipSizeY /= 2;

		if (mipSizeX < 2 || mipSizeY < 2) 
			break;

		BloomMip mip;
		mip.size = mipSize;
		mip.sizeX = mipSizeX;
		mip.sizeY = mipSizeY;
		///mip.texture = std::shared_ptr<Texture>(new Texture((unsigned int)mipSize.x, (unsigned int)mipSize.y, GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT));

		glGenTextures(1, &mip.texture);
		glBindTexture(GL_TEXTURE_2D, mip.texture);
		// we are downscaling an HDR color buffer, so we need a float texture format
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
			(int)mipSize.x, (int)mipSize.y,
			0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//std::cout << "Created bloom mip " << mipSizeX << 'x' << mipSizeY << std::endl;

		m_MipChain.emplace_back(mip);
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_MipChain[0].texture, 0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("gbuffer FBO error, status: 0x%x\n", status);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferBloom::Recalculate(const int& numMipsChain)
{
	m_NumColorAttachments = numMipsChain;
	Invalidate();
}
