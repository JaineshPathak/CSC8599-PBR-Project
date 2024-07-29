#include "TextureHDR.h"
#include <stb_image/stb_image.h>

TextureHDR::TextureHDR(const std::string& filePath) : Texture(filePath, false)
{
	Validate();
}

TextureHDR::~TextureHDR()
{
	glDeleteTextures(1, &m_ProgramID);
}

void TextureHDR::Validate()
{
	stbi_set_flip_vertically_on_load(true);
	m_Data = (float*)stbi_loadf(m_FilePath.c_str(), &m_Width, &m_Height, &m_Channel, 0);

	if (m_Data)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ProgramID);
		glBindTexture(GL_TEXTURE_2D, m_ProgramID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, m_Data);
		glBindTexture(GL_TEXTURE_2D, 0);

		if (m_Data)
			stbi_image_free(m_Data);

		stbi_set_flip_vertically_on_load(false);

		m_IsInitialized = true;
	}
	else
		m_IsInitialized = false;
}
