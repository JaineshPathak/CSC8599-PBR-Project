#include "TextureCubeMap.h"
#include <stb_image/stb_image.h>

TextureCubeMap::TextureCubeMap(const std::string& X_POS, const std::string& X_NEG, const std::string& Y_POS, const std::string& Y_NEG, const std::string& Z_POS, const std::string& Z_NEG) : Texture(0, 0)
{
	m_Texture_Faces_Files[0] = X_POS;
	m_Texture_Faces_Files[1] = X_NEG;
	m_Texture_Faces_Files[2] = Y_POS;
	m_Texture_Faces_Files[3] = Y_NEG;
	m_Texture_Faces_Files[4] = Z_POS;
	m_Texture_Faces_Files[5] = Z_NEG;

	Validate();
}

TextureCubeMap::TextureCubeMap(const std::string texture_faces[]) : Texture(0, 0)
{
	for (int i = 0; i < 6; i++)
		m_Texture_Faces_Files[i] = texture_faces[i];

	Validate();
}

TextureCubeMap::TextureCubeMap(const std::vector<std::string> texture_faces) : Texture(0, 0)
{
	for (int i = 0; i < 6; i++)
		m_Texture_Faces_Files[i] = texture_faces[i];

	Validate();
}

void TextureCubeMap::Validate()
{
	glGenTextures(1, &m_ProgramID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ProgramID);

	for (unsigned int i = 0; i < 6; i++)
	{
		//stbi_set_flip_vertically_on_load(true);
		m_Data = stbi_load(m_Texture_Faces_Files[i].c_str(), &m_Width, &m_Height, &m_Channel, 0);
		if (m_Data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_Data);
			stbi_image_free(m_Data);
		}
	}
	//stbi_set_flip_vertically_on_load(false);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	m_IsInitialized = true;
}
