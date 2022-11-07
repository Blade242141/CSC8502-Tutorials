#pragma once

#include "Plane.h"
class SceneNode;
class Matrix4;

class Frustum {
public:
	Frustum(void) {};
	~Frustum(void) {};

	void FromMatrix(Matrix4 const& mvp);
	bool InsideFrustum(SceneNode& n);

protected:
	Plane planes[6];
};
