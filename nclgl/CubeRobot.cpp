#include "CubeRobot.h"

SceneNode* CreatePart(Mesh* cube, Vector4 colour, Vector3 scale, Vector3 transform) {
	SceneNode* part = new SceneNode(cube, colour);
	part->SetModelScale(scale);
	part->SetTransform(Matrix4::Translation(transform));
	return part;
}


CubeRobot::CubeRobot(Mesh* cube) {
	//AddChild(CreatePart(cube, Vector4(1, 0, 0, 1), Vector3(10, 15, 5), Vector3(0, 35, 0))); // body
	//SceneNode* head = CreatePart(cube, Vector4(0, 1, 0, 1), Vector3(5, 5, 5), Vector3(0, 30, 0)); // head
	//AddChild(head);
	//SceneNode* leftArm = CreatePart(cube, Vector4(0, 0, 1, 1), Vector3(3, -18, 3), Vector3(-12, 30, -1)); // left arm
	//AddChild(leftArm);
	//SceneNode* rightArm = CreatePart(cube, Vector4(0, 0, 1, 1), Vector3(3, -18.5, 3), Vector3(12, 30, -1)); // right arm
	//AddChild(rightArm);
	//AddChild(CreatePart(cube, Vector4(0, 0, 1, 1), Vector3(3, -17.5, 3), Vector3(-8, 0, 0))); // left leg
	//AddChild(CreatePart(cube, Vector4(0, 0, 1, 1), Vector3(3, -17.5, 3), Vector3(8, 0, 0))); // right leg
	SceneNode * body = new SceneNode(cube, Vector4(1, 0, 0, 1)); //Red!
	body->SetModelScale(Vector3(10, 15, 5));
	body->SetTransform(Matrix4::Translation(Vector3(0, 35, 0)));
	AddChild(body);
	
	head = new SceneNode(cube, Vector4(0, 1, 0, 1)); // Green!
	head->SetModelScale(Vector3(5, 5, 5));
	head->SetTransform(Matrix4::Translation(Vector3(0, 30, 0)));
	body->AddChild(head);
	
	leftArm = new SceneNode(cube, Vector4(0, 0, 1, 1)); //Blue!
	leftArm->SetModelScale(Vector3(3, -18, 3));
	leftArm->SetTransform(Matrix4::Translation(Vector3(-12, 30, -1)));
	body->AddChild(leftArm);
	
	rightArm = new SceneNode(cube, Vector4(0, 0, 1, 1)); //Blue!
	rightArm->SetModelScale(Vector3(3, -18, 3));
	rightArm->SetTransform(Matrix4::Translation(Vector3(12, 30, -1)));
	body->AddChild(rightArm);
	
	SceneNode * leftLeg = new SceneNode(cube, Vector4(0, 0, 1, 1)); //Blue!
	leftLeg->SetModelScale(Vector3(3, -17.5, 3));
	leftLeg->SetTransform(Matrix4::Translation(Vector3(-8, 0, 0)));
	body->AddChild(leftLeg);
	
	SceneNode * rightLeg = new SceneNode(cube, Vector4(0, 0, 1, 1)); //Blue!
	rightLeg->SetModelScale(Vector3(3, -17.5, 3));
	rightLeg->SetTransform(Matrix4::Translation(Vector3(8, 0, 0)));
	body->AddChild(rightLeg);
	}

void CubeRobot::Update(float dt) {
	transform = transform * Matrix4::Rotation(30.0f * dt, Vector3(0, 1, 0));

	head->SetTransform(head->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(0, 1, 0)));
	leftArm->SetTransform(leftArm->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(1, 0, 0)));
	rightArm->SetTransform(rightArm->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(0, 0, 1)));

	SceneNode::Update(dt);
}