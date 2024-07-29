#pragma once
#include <iostream>
#include <string>
#include <glad/glad.h>

struct TextureData
{
	std::string path;
	unsigned char* data;
	int texWidth;
	int texHeight;
	int texChannels;

	TextureData()
	{
		path = "";
		data = nullptr;
		texWidth = 0;
		texHeight = 0;
		texChannels = 0;
	}

	TextureData(const std::string& fPath) : path(fPath)
	{
		data = nullptr;
		texWidth = 0;
		texHeight = 0;
		texChannels = 0;
	}
};

class Texture
{
public:
	Texture(const unsigned int& width, const unsigned int& height, bool generateMipMaps = true);
	Texture(const unsigned int& width, const unsigned int& height, const int& internalFormat, const int& normalFormat, bool generateMipMaps = true);
	Texture(const unsigned int& width, const unsigned int& height, const int& internalFormat, const int& normalFormat, const int& type, bool generateMipMaps = true);
	Texture(const unsigned int& width, const unsigned int& height, const int& internalFormat, const int& normalFormat, const int& type, const int& minFilter, const int& magFilter, const int& wrapMode, bool generateMipMaps = true);
	Texture(const std::string& filePath, const unsigned int& width, const unsigned int& height, bool generateMipMaps = true);
	Texture(const std::string& filePath, const int& internalFormat, const int& normalFormat, const int& type, const int& minFilter, const int& magFilter, const int& wrapMode, bool generateMipMaps = true);
	Texture(const std::string& filePath, bool shouldValidate = true);
	Texture(const TextureData& texData);
	virtual ~Texture();

	unsigned int GetID() const { return m_ProgramID; }
	const int GetWidth() const { return m_Width; }
	const int GetHeight() const { return m_Height; }
	const int GetChannels() const { return m_Channel; }
	const void* GetData() const { return m_Data; };

	inline virtual void Bind();
	inline virtual void Unbind();
	virtual void UploadData(void* data);

	bool IsInitialized() const
	{ 
		if (!m_IsInitialized)
			std::cout << "Texture: " << m_FilePath << " is not initialized! Program ID: " << m_ProgramID << std::endl;

		return m_IsInitialized; 
	}

protected:
	virtual void Validate();

	bool m_IsInitialized;
	unsigned int m_ProgramID;

	std::string m_FilePath;
	int m_Width, m_Height, m_Channel;
	int m_InternalFormat;
	int m_Format;
	int m_Type;
	bool m_IsMipMapped;
	void* m_Data;
};