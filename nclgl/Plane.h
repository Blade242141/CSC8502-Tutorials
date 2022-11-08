#pragma once
#include "Vector3.h"

class Plane {
public:
	Plane(void) {};
	Plane(Vector3 const& normal, float distance, bool normalise = false);
	~Plane(void) {};

	void SetNormal(Vector3 const& normal) { this->normal = normal; }
	Vector3 const GetNormal() { return normal; }

	void SetDistance(float dist) { distance = dist; }
	float const GetDistance() { return distance; }

	bool const SphereInPlane(const Vector3& position, float radius);

protected:
	Vector3 normal;
	float distance;
};