#include "LookAtCamera.h"

#include <nclgl/Window.h>
#include <imgui/imgui.h>

const float lerpTime = 2.0f;
float lerpCounter = 0.0f;

LookAtCamera::LookAtCamera() :
	m_lookAtDistance(0.0f), m_Sensitivity(6.0f), m_RotationSpeed(10.0f), m_CameraMovementType(0), Camera()
{
	SetFOV(60.0f);
	SetZNear(0.1f);
	ImGuiRenderer::Get()->RegisterItem(this);
}

LookAtCamera::LookAtCamera(float _pitch, float _yaw, float _roll, Vector3 _position) :
	m_lookAtDistance(0.0f), m_Sensitivity(6.0f), m_RotationSpeed(10.0f), m_CameraMovementType(0), Camera(_pitch, _yaw, _roll, _position)
{
	SetFOV(60.0f);
	SetZNear(0.1f);
	ImGuiRenderer::Get()->RegisterItem(this);
}

LookAtCamera::LookAtCamera(Vector3 _position, Vector3 _rotation) :
	m_lookAtDistance(0.0f), m_Sensitivity(6.0f), m_RotationSpeed(10.0f), m_CameraMovementType(0), Camera(_position, _rotation)
{
	SetFOV(60.0f);
	SetZNear(0.1f);
	ImGuiRenderer::Get()->RegisterItem(this);
}

void LookAtCamera::UpdateCamera(float dt)
{
	if (ImGui::GetCurrentContext() == nullptr) return;
	//if (ImGui::GetIO().MouseDrawCursor) return;
	//if (ImGui::GetCurrentContext()->HoveredWindow != nullptr && ImGui::GetCurrentContext()->HoveredWindow->Name == "Scene") return;
	//if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsAnyItemFocused()) return;

	//std::cout << "Mouse Over Scene: " << ImGuiRenderer::Get()->IsMouseOverScene() << std::endl;

	if (m_CameraMovementType == 1)
	{
		Camera::UpdateCamera(dt);
		return;
	}

	if (Window::GetMouse()->ButtonHeld(MouseButtons::MOUSE_LEFT) && ImGuiRenderer::Get()->IsMouseOverScene())
	{
		m_CamRotation.x -= (Window::GetMouse()->GetRelativePosition().y) * m_Sensitivity;
		m_CamRotation.y -= (Window::GetMouse()->GetRelativePosition().x) * m_Sensitivity;

		m_CamRotation.x = std::min(m_CamRotation.x, 89.0f);
		m_CamRotation.x = std::max(m_CamRotation.x, -89.0f);

	}
	else
	{
		if(m_RotationSpeed > 0.001f)
			m_CamRotation.y += m_RotationSpeed * dt;
	}

	if (m_CamRotation.y < 0)		m_CamRotation.y += 360.0f;
	if (m_CamRotation.y > 360.0f)	m_CamRotation.y -= 360.0f;

	Matrix4 yawMat = Matrix4::Rotation(m_CamRotation.y, Vector3::UP);
	Matrix4 pitchMat = Matrix4::Rotation(m_CamRotation.x, yawMat * Vector3::RIGHT);
	Matrix4 finalRotMat = pitchMat * yawMat;

	m_CamFront = finalRotMat * Vector3::FORWARD;
	m_CamUp = finalRotMat * Vector3::UP;
	m_CamRight = finalRotMat * Vector3::RIGHT;
	
	Vector3 lookDirection = finalRotMat * Vector3::FORWARD;
	Vector3 finalPosition = m_lookAtPos - lookDirection * m_lookAtDistance;	

	m_CamPosition = finalPosition;
}

Matrix4 LookAtCamera::CalcViewMatrix()
{
	if (m_CameraMovementType == 1)
		return Camera::CalcViewMatrix();

	m_CamViewMat = Matrix4::BuildViewMatrix(GetPosition(), m_lookAtPos);
	return m_CamViewMat;
}

void LookAtCamera::OnImGuiRender()
{	
	if (ImGui::CollapsingHeader("Camera"))
	{
		ImGui::Indent();

		ImGui::Combo("Movement Type", &m_CameraMovementType, m_CameraMovementTypeStr, 2);

		if (m_CameraMovementType == 0)
		{
			float m_speed = m_RotationSpeed;
			if (ImGui::SliderFloat("Rotation Speed", &m_speed, 0.0f, 30.0f)) m_RotationSpeed = m_speed;
		}
		else if (m_CameraMovementType == 1)
		{
			float m_Speed = m_DefaultSpeed;
			if (ImGui::SliderFloat("Speed", &m_Speed, 1.0f, 20.0f)) m_DefaultSpeed = m_Speed;
		}

		ImGui::Separator();

		float m_sens = m_Sensitivity;
		if (ImGui::SliderFloat("Sensitivity", &m_sens, 1.0f, 8.0f)) m_Sensitivity = m_sens;

		float m_fov = m_FOV;
		if (ImGui::SliderFloat("FOV", &m_fov, 10.0f, 80.0f)) SetFOV(m_fov);

		float m_lookDist = m_lookAtDistance;
		if (ImGui::SliderFloat("Look At Distance", &m_lookDist, 2.0f, 10.0f)) SetLookAtDistance(m_lookDist);		
		
		ImGui::Separator();
		
		float m_camPos[3] = { m_CamPosition.x, m_CamPosition.y, m_CamPosition.z };
		ImGui::DragFloat3("Position", m_camPos);

		float m_camRot[3] = { m_CamRotation.x, m_CamRotation.y, m_CamRotation.z };
		ImGui::DragFloat3("Rotation", m_camRot);

		float m_camUpDir[3] = { m_CamUp.x, m_CamUp.y, m_CamUp.z };
		ImGui::DragFloat3("Up", m_camUpDir);

		float m_camFrontDir[3] = { m_CamFront.x, m_CamFront.y, m_CamFront.z };
		ImGui::DragFloat3("Front", m_camFrontDir);

		float m_camRightDir[3] = { m_CamRight.x, m_CamRight.y, m_CamRight.z };
		ImGui::DragFloat3("Right", m_camRightDir);

		ImGui::Unindent();
	}
}