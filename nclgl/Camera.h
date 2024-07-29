#pragma once
#include "Matrix4.h"
#include "Vector3.h"

class Camera
{
public:
	Camera(void)
	{
		m_CamPosition.ToZero();

		m_CamRotation.x = 0.0f;
		m_CamRotation.y = 0.0f;
		m_CamRotation.z = 0.0f;

		m_CamFront = Vector3::FORWARD;
		m_CamUp = Vector3::UP;
		m_CamRight = Vector3::RIGHT;

		m_CurrentSpeed = 0.0f;

		m_FOV = 45.0f;
		m_AspectRatio = 0.0f;

		m_ZNear = 1.0f;
		m_ZFar = 1000.0f;
	}

	Camera(float _pitch, float _yaw, float _roll, Vector3 _position)
	{
		m_CamPosition = _position;

		m_CamRotation.x = _pitch;
		m_CamRotation.y = _yaw;
		m_CamRotation.z = _roll;

		m_CamFront = Vector3::FORWARD;
		m_CamUp = Vector3::UP;
		m_CamRight = Vector3::RIGHT;

		m_CurrentSpeed = 0.0f;
		m_FOV = 45.0f;
		m_AspectRatio = 0.0f;

		m_ZNear = 1.0f;
		m_ZFar = 1000.0f;
	}

	Camera(Vector3 _position, Vector3 _rotation)
	{
		//yaw = _yaw;
		//pitch = _pitch;
		m_CamPosition = _position;
		m_CamRotation = _rotation;

		m_CamFront = Vector3::FORWARD;
		m_CamUp = Vector3::UP;
		m_CamRight = Vector3::RIGHT;

		m_CurrentSpeed = 0.0f;
		m_FOV = 45.0f;
		m_AspectRatio = 0.0f;

		m_ZNear = 1.0f;
		m_ZFar = 1000.0f;
	}

	~Camera(void) {}

	float GetDefaultSpeed() const { return m_DefaultSpeed; }
	inline void SetDefaultSpeed(float s) { m_DefaultSpeed = s; }

	virtual void UpdateCamera(float dt = 1.0f);
	
	virtual Matrix4 CalcViewMatrix();
	virtual Matrix4 GetViewMatrix() const { return m_CamViewMat; }

	virtual void CalcProjectionMatrix();
	virtual Matrix4 GetProjectionMatrix() const { return m_CamProjMat; }

	Vector3 GetPosition() const { return m_CamPosition; }
	void SetPosition(const Vector3& val) { m_CamPosition = val; }

	Vector3 GetRotation() const { return m_CamRotation; }
	void SetRotation(const Vector3& val) { m_CamRotation = val; }

	Vector3 GetUp() const { return m_CamUp; }
	Vector3 GetForward() const { return m_CamFront; }
	Vector3 GetRight() const { return m_CamRight; }

	float GetPitch() const { return m_CamRotation.x; }
	void SetPitch(float p) { m_CamRotation.x = p; }

	float GetYaw() const { return m_CamRotation.y; }
	void SetYaw(float val) { m_CamRotation.y = val; }

	float GetRoll() const { return m_CamRotation.z; }
	void SetRoll(float val) { m_CamRotation.z = val; }

	float GetZNear() const { return m_ZNear; }
	void SetZNear(const float& newZ) { m_ZNear = newZ; CalcProjectionMatrix(); }

	float GetZFar() const { return m_ZFar; }
	void SetZFar(const float& newZ) { m_ZFar = newZ; CalcProjectionMatrix(); }

	float GetFOV() const { return m_FOV; }
	void SetFOV(const float& newFOV) { m_FOV = newFOV; CalcProjectionMatrix(); }

	float GetAspectRatio() const { return m_AspectRatio; }
	void SetAspectRatio(const float& width, const float& height) { m_AspectRatio = width / height; CalcProjectionMatrix(); }

protected:
	float m_CurrentSpeed;
	float m_DefaultSpeed = 30.0f;

	float m_ZNear;
	float m_ZFar;

	float m_FOV;
	float m_AspectRatio;

	Vector3 m_CamPosition;
	Vector3 m_CamRotation;

	Vector3 m_CamFront;
	Vector3 m_CamUp;
	Vector3 m_CamRight;

	Matrix4 m_CamProjMat;
	Matrix4 m_CamViewMat;
};