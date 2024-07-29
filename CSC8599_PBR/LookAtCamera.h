#pragma once
#include "..\nclgl\Camera.h"
#include "IImguiItem.h"
#include "ImGuiRenderer.h"

class LookAtCamera : public Camera, public IImguiItem
{
public:
	LookAtCamera();
	LookAtCamera(float _pitch, float _yaw, float _roll, Vector3 _position);
	LookAtCamera(Vector3 _position, Vector3 _rotation);

	void SetLookAtPosition(const Vector3& _lookPos) { m_lookAtPos = _lookPos; }
	void SetLookAtDistance(const float& _lookDist) { m_lookAtDistance = _lookDist; }

	void UpdateCamera(float dt = 1.0f) override;
	Matrix4 CalcViewMatrix() override;

	virtual void OnImGuiRender() override;

protected:
	float m_lookAtDistance;
	float m_Sensitivity;
	float m_RotationSpeed;
	Vector3 m_lookAtPos;

	int m_CameraMovementType;
	const char* const m_CameraMovementTypeStr[2] { "Focused", "Free" };
};