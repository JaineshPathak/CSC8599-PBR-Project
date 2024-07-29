#pragma once
#include "..\nclgl\SceneNode.h"
#include "..\nclgl\common.h"

class CubeRobot : public SceneNode
{
public:
	CubeRobot(Mesh* cube);
	~CubeRobot(void) {};

	virtual void Update(float dt) override;

protected:
	SceneNode* head;
	SceneNode* leftArm;
	SceneNode* rightArm;
	SceneNode* hip;
	SceneNode* body;
	SceneNode* leftLeg;
	SceneNode* rightLeg;

	float inc;
	float timeElapsed = 0.0f;
	float xRotSpeed = 45.0f;
	bool toggleHand;

	Matrix4 origBodyTransform;
};