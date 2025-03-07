#pragma once
#include "Matrix4.h"
#include "Matrix3.h"

class Camera {
public:
	Camera(void) {
		yaw = 0.0f;
		pitch = 0.0f;
	};

	Camera(float pitch, float yaw, Vector3 position) {
		this->pitch = pitch;
		this->yaw = yaw;
		this->position = position;
	}

	~Camera(void) {};

	void UpdateCamera(float dt = 1.0f, bool move = true);

	Matrix4 BuildViewMatrix();

	Vector3 const GetPosition() { return position; }
	void SetPosition(Vector3 val) { position = val; }

	float const GetYaw() { return yaw; }
	void SetYaw(float y) { yaw = y; }

	float const GetPitch() { return pitch; }
	void SetPitch(float p) { pitch = p; }

	void MoveTowards(Vector3 v, float maxDistanceDelta) {
		Vector3 a = v - position;
		float magnitude = (a.x * a.x + a.y * a.y + a.z * a.z);
		if (magnitude <= maxDistanceDelta || magnitude == 0.0f)
			position = v;
		else
			position = position + a / magnitude * maxDistanceDelta;
	}


protected:
	float yaw;
	float pitch;
	Vector3 position;
};