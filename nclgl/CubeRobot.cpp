#include "CubeRobot.h"

CubeRobot::CubeRobot(Mesh* cube)
{
	body = new SceneNode(cube, Vector4(1, 0, 0, 1));
	body->SetModelScale(Vector3(10, 15, 5));
	origBodyTransform = Matrix4::Translation(Vector3(0, 35, 0));
	body->SetTransform(origBodyTransform);
	body->nodeName = "Body";
	body->SetBoundingRadius(15.0f);
	AddChild(body);

	head = new SceneNode(cube, Vector4(0, 1, 0, 1));
	head->SetModelScale(Vector3(5, 5, 5));
	head->SetTransform(Matrix4::Translation(Vector3(0, 30, 0)));
	head->nodeName = "Head";
	head->SetBoundingRadius(5.0f);
	body->AddChild(head);

	leftArm = new SceneNode(cube, Vector4(0, 0, 1, 1));
	leftArm->SetModelScale(Vector3(3, -18, 3));
	leftArm->SetTransform(Matrix4::Translation(Vector3(-13, 30, -1)));
	leftArm->nodeName = "leftArm";
	leftArm->SetBoundingRadius(18.0f);
	body->AddChild(leftArm);

	rightArm = new SceneNode(cube, Vector4(0, 0, 1, 1));
	rightArm->SetModelScale(Vector3(3, -18, 3));
	rightArm->SetTransform(Matrix4::Translation(Vector3(13, 30, -1)));
	rightArm->nodeName = "rightArm";
	rightArm->SetBoundingRadius(18.0f);
	body->AddChild(rightArm);

	//-------------------------------------------------

	hip = new SceneNode(cube, Vector4(1, 1, 1, 1));
	hip->SetModelScale(Vector3(1, 1, 1));
	hip->SetTransform(Matrix4::Translation(Vector3(0, 35, 0)));
	hip->SetBoundingRadius(18.0f);
	AddChild(hip);

	leftLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));
	leftLeg->SetModelScale(Vector3(3, -17.5, 3));
	leftLeg->SetTransform(Matrix4::Translation(Vector3(-8, 0, 0)));
	leftLeg->SetBoundingRadius(18.0f);
	hip->AddChild(leftLeg);

	rightLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));
	rightLeg->SetModelScale(Vector3(3, -17.5, 3));
	rightLeg->SetTransform(Matrix4::Translation(Vector3(8, 0, 0)));
	rightLeg->SetBoundingRadius(18.0f);
	hip->AddChild(rightLeg);
}

void CubeRobot::Update(float dt)
{
	//Rotate the whole body
	//localTransform = localTransform * Matrix4::Rotation(30.0f * dt, Vector3(0, 1, 0));

	//body->SetTransform(body->GetTransform() * Matrix4::Rotation(-90.0f * dt, Vector3(1, 0, 0)));

	//if (timeElapsed <= 3.0f)
	timeElapsed += dt;
	xRotSpeed = naive_lerp(-45.0f, 45.0f, (sin(timeElapsed) + 1.0f) / 2.0f);
	body->SetTransform(origBodyTransform * Matrix4::Rotation(xRotSpeed, Vector3(1, 0, 0)));

	head->SetTransform(head->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(0, 1, 0)));
	leftArm->SetTransform(leftArm->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(1, 0, 0)));
	rightArm->SetTransform(rightArm->GetTransform() * Matrix4::Rotation(30.0f * dt, Vector3(1, 0, 0)));

	SceneNode::Update(dt);
}