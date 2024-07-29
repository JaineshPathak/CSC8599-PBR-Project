#include "SceneNode.h"

SceneNode::SceneNode(Mesh* m, Vector4 col)
{
	mesh = m;
	colour = col;
	parent = NULL;
	modelScale = Vector3(1, 1, 1);
	modelRotation = Vector3(0, 0, 0);

	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
}

SceneNode::~SceneNode(void)
{
	for (size_t i = 0; i < children.size(); i++)
		delete children[i];
}

void SceneNode::AddChild(SceneNode* s)
{
	children.push_back(s);
	s->parent = this;
}

SceneNode* SceneNode::GetChild(int index)
{
	if (index < 0 || index > children.size())
		return NULL;

	return children[index];
}

void SceneNode::Draw(const OGLRenderer& r)
{
	if (mesh)
		mesh->Draw();
}

void SceneNode::CalcBoundingBox()
{
	if (!mesh)
		return;

	Matrix4 modelMatrix = worldTransform * Matrix4::Scale(modelScale);
	const Vector3 globalCenter = modelMatrix * mesh->GetBoundingBoxCenter();
	const Vector3 boundingBoxExtents = mesh->GetBoundingBoxExtents();

	Matrix4 localMatrix = modelMatrix;
	localMatrix.SetPositionVector(Vector3(0, 0, 0));

	Vector3 nodeRight = localMatrix * Vector3(1.0f, 0.0f, 0.0f);
	Vector3 nodeUp = localMatrix * Vector3(0.0f, 1.0f, 0.0f);
	Vector3 nodeForward = localMatrix * Vector3(0.0f, 0.0f, -1.0f);

	Vector3 right = nodeRight * boundingBoxExtents.x;
	Vector3 up = nodeUp * boundingBoxExtents.y;
	Vector3 forward = nodeForward * boundingBoxExtents.z;

	const float newIi = std::abs(Vector3::Dot(Vector3(1.f, 0.f, 0.f), right)) +
		std::abs(Vector3::Dot(Vector3(1.f, 0.f, 0.f), up)) +
		std::abs(Vector3::Dot(Vector3(1.f, 0.f, 0.f), forward));

	const float newIj = std::abs(Vector3::Dot(Vector3(0.f, 1.f, 0.f), right)) +
		std::abs(Vector3::Dot(Vector3(0.f, 1.f, 0.f), up)) +
		std::abs(Vector3::Dot(Vector3(0.f, 1.f, 0.f), forward));

	const float newIk = std::abs(Vector3::Dot(Vector3(0.f, 0.f, -1.f), right)) +
		std::abs(Vector3::Dot(Vector3(0.f, 0.f, -1.f), up)) +
		std::abs(Vector3::Dot(Vector3(0.f, 0.f, -1.f), forward));

	nodeBoundingBoxCenter = globalCenter;
	nodeBoundingBoxExtents = Vector3(newIi, newIj, newIk);
}

void SceneNode::Update(float dt)
{
	worldTransform = parent ? parent->worldTransform * localTransform : localTransform;
	CalcBoundingBox();

	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); ++i)
		(*i)->Update(dt);
}