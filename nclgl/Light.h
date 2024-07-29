#pragma once

#include "Vector3.h"
#include "Vector4.h"

enum LIGHT_TYPE
{
	TYPE_POINTLIGHT = 0,
	TYPE_DIRECTIONALLIGHT = 1,
	TYPE_SPOTLIGHT = 2
};

class Light
{
public:
	Light() = delete;
	Light(const Vector3& position, const Vector4& colour, float radius)
	{
		this->lightType = LIGHT_TYPE::TYPE_POINTLIGHT;
		this->position = position;
		this->colour = colour;
		this->radius = radius;
	}

	virtual ~Light(void) {}

	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& val) { position = val; }

	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4& val) { colour = val; }

	float GetRadius() const { return radius; }
	void SetRadius(float val) { radius = val; }

	float GetIntensity() const { return intensity; }
	void SetIntensity(float val) { intensity = val; }

protected:
	LIGHT_TYPE lightType;
	Vector3 position;
	Vector4 colour;
	float radius;
	float intensity = 1.0f;
};