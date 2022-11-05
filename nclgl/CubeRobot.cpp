#include "CubeRobot.h"

CubeRobot::CubeRobot(Mesh* cube) {
	AddChild(CreatePart(cube, Vector4(1, 0, 0, 1), Vector3(10, 15, 5), Vector3(0, 35, 0))); // body
	SceneNode* head = CreatePart(cube, Vector4(0, 1, 0, 1), Vector3(5, 5, 5), Vector3(0, 30, 0)); // head
	AddChild(head);
	SceneNode* leftArm = CreatePart(cube, Vector4(0, 0, 1, 1), Vector3(3, -18, 3), Vector3(-12, 30, -1)); // left arm
	AddChild(leftArm);
	SceneNode* rightArm = CreatePart(cube, Vector4(0, 0, 1, 1), Vector3(3, -18.5, 3), Vector3(12, 30, -1)); // right arm
	AddChild(rightArm);
	AddChild(CreatePart(cube, Vector4(0, 0, 1, 1), Vector3(3, -17.5, 3), Vector3(-8, 0, 0))); // left leg
	AddChild(CreatePart(cube, Vector4(0, 0, 1, 1), Vector3(3, -17.5, 3), Vector3(8, 0, 0))); // right leg

}

SceneNode* CreatePart(Mesh* cube, Vector4 colour, Vector3 scale, Vector3 transform) {
	SceneNode* part = new SceneNode(cube, colour);
	part->SetModelScale(scale);
	part->SetTransform(Matrix4::Translation(transform));
	return part;
}

void CubeRobot::Update(float dt) {
	transform = transform * Matrix4::Rotation(30.0f * dt, Vector3(0, 1, 0));

	head->SetTransform(head->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(0, 1, 0)));
	leftArm->SetTransform(rightArm->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(1, 0, 0)));
	rightArm->SetTransform(rightArm->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(0, 0, 1)));

	SceneNode::Update(dt);
}