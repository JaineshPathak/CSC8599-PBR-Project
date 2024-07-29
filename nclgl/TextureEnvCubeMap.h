#pragma once
#include "Texture.h"

class TextureEnvCubeMap : public Texture
{
public:
	TextureEnvCubeMap(const unsigned int width, const unsigned int height, bool generateMipMaps = false);

protected:
	virtual void Validate() override;

	bool m_ShouldGenerateMipMaps;	
};