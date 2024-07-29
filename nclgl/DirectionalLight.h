#pragma once

#include "Light.h"
#include "Vector3.h"

class DirectionalLight : public Light
{
public:
	DirectionalLight(const Vector3& lightDir, const Vector4& lightColour) :
		Light(Vector3(0.0f, 0.0f, 0.0f), lightColour, 0.0f)
	{
		this->lightType = TYPE_DIRECTIONALLIGHT;		
		this->lightDir = lightDir;			
	}
	~DirectionalLight() {};

	inline Vector3 GetLightDir() const { return lightDir; }
	inline void SetLightDir(const Vector3& dir) { lightDir = dir; }

protected:
	Vector3 lightDir;
};