#pragma once

#include "Light.h"
#include "Vector3.h"

class SpotLight : public Light
{
public:
	SpotLight(const Vector3& lightPos, const Vector3& lightDir, const Vector4& lightColor, const float& innerCutOff, const float& outerCutOff) :
		Light(lightPos, lightColor, 0.0f)
	{
		this->innerCutOff = innerCutOff;
		this->outerCutOff = outerCutOff;
		this->lightType = TYPE_SPOTLIGHT;
		this->lightDir = lightDir;
	}	
	~SpotLight() {};

	inline Vector3 GetLightDir() const { return lightDir; }
	inline void SetLightDir(const Vector3& dir) { lightDir = dir; }

	inline float GetInnerCutOff() const { return innerCutOff; }
	inline void SetInnerCutOff(const float& newInnerCutOff) { innerCutOff = newInnerCutOff; }

	inline float GetOuterCutOff() const { return outerCutOff; }
	inline void SetOuterCutOff(const float& newOuterCutOff) { outerCutOff = newOuterCutOff; }

protected:
	Vector3 lightDir;
	float innerCutOff;
	float outerCutOff;
};