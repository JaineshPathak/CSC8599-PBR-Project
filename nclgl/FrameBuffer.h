#pragma once

#include "Texture.h"
#include "IObserver.h"
#include <glad/glad.h>
#include <vector>
#include <memory>

class FrameBuffer : public IObserver
{
public:
	FrameBuffer();
	FrameBuffer(const unsigned int& sizeX, const unsigned int& sizeY, const int& numColorAttachments = 1);
	FrameBuffer(const unsigned int& sizeX, const unsigned int& sizeY, const int& colorAttachmentInternalFormat, const int& colorAttachmentNormalFormat, const int& colorAttachmentType, const int& numColorAttachments = 1);
	FrameBuffer(const unsigned int& sizeX, const unsigned int& sizeY, const int& colorAttachmentInternalFormat, const int& colorAttachmentNormalFormat, const int& colorAttachmentType, const unsigned int& minFilter, const unsigned int& magFilter, const unsigned int& wrapMode, const int& numColorAttachments = 1, const bool& allowColorAttachmentMipMaps = true);
	virtual ~FrameBuffer();

protected:
	virtual void Invalidate();

public:
	virtual void Bind();
	virtual void Unbind();
	virtual void Destroy();

	virtual void UpdateData(const int& sizeX, const int& sizeY) override;

	void Resize(const unsigned int& new_width, const unsigned int& new_height);

	virtual void AddNewColorAttachment();
	virtual void AttachExistingColorAttachment(const unsigned int& texID, const int& attachSlot = 0);

	virtual void AttachExistingDepthAttachment(const unsigned int& texID, const unsigned int& depthComponentType, const bool& doBinding = true);
	virtual void RemoveDepthAttachment(const bool& doBinding = true);

	const unsigned int GetID() const { return m_ProgramID; }
	const unsigned int GetColorAttachmentTex(const int& index = 0) const;
	const unsigned int GetDepthAttachmentTex() const;

	const unsigned int GetWidth() const { return m_Width; }
	const unsigned int GetHeight() const { return m_Height; }

	const int GetNumColorAttachments() const { return m_NumColorAttachments; }

	void SetDeleteDepthTextureStatus(const bool& status) { m_ShouldDeleteDepthTexture = status; }

protected:
	unsigned int m_ProgramID;
	//unsigned int m_ColorAttachmentTex = 0, m_DepthAttachmentTex = 0;
	std::shared_ptr<Texture> m_DepthAttachmentTex;
	std::vector<std::shared_ptr<Texture>> m_ColorAttachedTextures;

	unsigned int m_Width, m_Height;
	int m_NumColorAttachments;
	int m_ColorAttachmentsInternalFormat;
	int m_ColorAttachmentsNormalFormat;
	int m_ColorAttachmentsType;	//Eg: GL_FLOAT, GL_UNSIGNED_INT
	int m_ColorAttachmentMinFilter;
	int m_ColorAttachmentMagFilter;
	int m_ColorAttachmentWrapMode;
	bool m_ColorAttachmentMipMapsEnabled;
	bool m_ShouldDeleteDepthTexture;
};