#include "FrameBufferFP.h"
#include <iostream>

void FrameBufferFP::Invalidate()
{
	if (m_ProgramID)
	{
		Destroy();
	}

	glCreateFramebuffers(1, &m_ProgramID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ProgramID);

	m_DepthAttachmentTex = std::shared_ptr<Texture>(new Texture(m_Width, m_Height, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, false));
	AttachExistingDepthAttachment(m_DepthAttachmentTex->GetID(), GL_DEPTH_ATTACHMENT, false);
	
	if (m_ShouldDeleteDepthTexture)
		RemoveDepthAttachment(false);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Frame Buffer is Incomplete! ID: " << m_ProgramID << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
