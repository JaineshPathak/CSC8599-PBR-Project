#pragma once
#include "Vector3.h"

class Plane
{
public:
	Plane(void) {};
	Plane(const Vector3& normal, float distance, bool normalise = false);
	~Plane(void) {};

	Vector3 GetNormal() const { return normal; }
	void SetNormal(const Vector3& normal) { this->normal = normal; };

	void SetDistance(float dist) { distance = dist; }
	float GetDistance() const { return distance; }

	bool SphereInPlane(const Vector3& position, float radius) const;
	bool BoxInPlane(const Vector3& position, const Vector3& extents) const;

protected:
	Vector3 normal;
	float distance;
};