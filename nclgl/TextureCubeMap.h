#pragma once
#include "Texture.h"
#include <vector>

class TextureCubeMap : public Texture
{
public:
	TextureCubeMap(const std::string& X_POS, const std::string& X_NEG,
					const std::string& Y_POS, const std::string& Y_NEG,
					const std::string& Z_POS, const std::string& Z_NEG);
	TextureCubeMap(const std::string texture_faces[]);
	TextureCubeMap(const std::vector<std::string> texture_faces);

protected:
	virtual void Validate() override;
	
	std::string m_Texture_Faces_Files[6];
};