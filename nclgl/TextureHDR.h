#pragma once
#include "Texture.h"

class TextureHDR : public Texture
{
public:
	TextureHDR(const std::string& filePath);
	~TextureHDR();	

protected:
	virtual void Validate();	
};