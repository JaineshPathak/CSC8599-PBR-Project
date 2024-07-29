#include "Camera.h"
#include "Window.h"
#include <algorithm>
#include <imgui/imgui.h>

void Camera::UpdateCamera(float dt)
{
	m_CamRotation.x -= (Window::GetMouse()->GetRelativePosition().y) * 2.0f;
	m_CamRotation.y -= (Window::GetMouse()->GetRelativePosition().x) * 2.0f;

	m_CamRotation.x = std::min(m_CamRotation.x, 90.0f);
	m_CamRotation.x = std::max(m_CamRotation.x, -90.0f);

	if (m_CamRotation.y < 0)		m_CamRotation.y += 360.0f;
	if (m_CamRotation.y > 360.0f)	m_CamRotation.y -= 360.0f;

	Matrix4 rotation = Matrix4::Rotation(m_CamRotation.y, Vector3::UP) * Matrix4::Rotation(m_CamRotation.x, Vector3::RIGHT);
	m_CamFront = rotation * Vector3::FORWARD;
	m_CamUp = rotation * Vector3::UP;
	m_CamRight = rotation * Vector3::RIGHT;

	/*camViewMat = Matrix4::BuildViewMatrix(camPosition, camPosition + camFront, camUp);

	Vector3 lookDir;
	lookDir.x = cos(DegToRad(camRotation.y)) * cos(DegToRad(camRotation.x));
	lookDir.y = sin(DegToRad(camRotation.x));
	lookDir.z = sin(DegToRad(camRotation.y)) * cos(DegToRad(camRotation.x));
	camFront = lookDir.Normalised();
	camRight = Vector3::Cross(camFront, camUp).Normalised();*/

	m_CurrentSpeed = m_DefaultSpeed * dt;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W))			m_CamPosition += m_CamFront * m_CurrentSpeed;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S))			m_CamPosition -= m_CamFront * m_CurrentSpeed;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A))			m_CamPosition -= m_CamRight * m_CurrentSpeed;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D))			m_CamPosition += m_CamRight * m_CurrentSpeed;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT))		m_CurrentSpeed *= 2.0f;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE))		m_CamPosition.y += m_CurrentSpeed;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_CONTROL))	m_CamPosition.y -= m_CurrentSpeed;	
}

Matrix4 Camera::CalcViewMatrix()
{
	m_CamViewMat = Matrix4::Rotation(-m_CamRotation.x, Vector3::RIGHT) * Matrix4::Rotation(-m_CamRotation.y, Vector3::UP) * Matrix4::Translation(-m_CamPosition);
	return m_CamViewMat;
	//return Matrix4::Rotation(-camRotation.x, Vector3(1, 0, 0)) * Matrix4::Rotation(-camRotation.y, Vector3(0, 1, 0)) * camViewMat;
	//return camViewMat;
}

void Camera::CalcProjectionMatrix()
{
	m_CamProjMat = Matrix4::Perspective(m_ZNear, m_ZFar, m_AspectRatio, m_FOV);
}