#pragma once

#include "Vector4.h"
#include "Vector3.h"

class Light {
public:
	Light() {}
	Light(const Vector3& pos, const Vector4& colour, float radius) {
		this->pos = pos;
		this->colour = colour;
		this->radius = radius;
	}

	~Light(void) {};

	Vector3 getPos()const { return pos; }
	void SetPos(const Vector3& val) { pos = val; }
	
	float GetRadius() const { return radius; }
	void SetRadious(float val) { radius = val; }

	Vector4 Getcolour() const { return colour; }
	void SetColour(const Vector4& val) { colour = val; }

protected:
	Vector3 pos;
	float radius;
	Vector4 colour;
};