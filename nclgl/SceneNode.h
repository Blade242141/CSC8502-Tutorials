#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include <vector>
#include "MeshMaterial.h"
#include "Shader.h"

class SceneNode {
public:
	SceneNode(Mesh* m = NULL, Vector4 colour = Vector4(1, 1, 1, 1));
	~SceneNode(void);

	//Tutorial 7
	float const GetBoundingRadius()  { return boundingRadius; }
	void SetBoundingRadius(float f) { boundingRadius = f; }

	float const GetCameraDistance() { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }

	void SetTexture(GLuint tex) { texture = tex; }
	GLuint const GetTexture() { return texture; }

	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b) {
		return (a->distanceFromCamera < b->distanceFromCamera) ? true : false;
	}

	//Tutorial 6
	void SetTransform(const Matrix4& matrix) { transform = matrix; }
	const Matrix4& GetTransform() const { return transform; }
	Matrix4 GetWorldTransform() const { return worldTransform; }

	Vector4 GetColour() const { return colour; }
	void SetColour(Vector4 c) { colour = c; }

	Vector3 GetModelScale() const { return modelScale; }
	void SetModelScale(Vector3 s) { modelScale = s; }
	Mesh* GetMesh() const { return mesh; }
	void SetMesh(Mesh* m) { mesh = m; }
	void AddChild(SceneNode* s);

	virtual void Update(float dt);
	virtual void Draw(const OGLRenderer& r);

	std::vector<SceneNode*>::const_iterator GetCHildIteratorStart() { return children.begin(); }

	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd() { return children.end(); }

	//Added
	MeshMaterial* GetMeshMaterial() const { return mat; }
	void SetMeshMaterial(MeshMaterial* m) { mat = m; }

	vector<GLuint> GetTextures() const { return textures; }
	void SetTxtures(vector<GLuint> v) { textures = v; }

	bool bump;
	GLuint GetBumpTextures() const { return bumpTexture; }
	void SetBumpTxtures(GLuint t) { bumpTexture = t; t != 0 ? bump = true : bump = false; }
	bool HasBump() const { return bump; }

	bool metal;
	GLuint GetMetalTexture() const { return metalnessTexture; }
	void SetMetalTexture(GLuint t) { metalnessTexture = t; t != 0 ? metal = true : metal = false; }
	bool HasMetal() const { return metal; }


	Shader* GetShader() const { return shader; }
	void SetShader(Shader* s) { shader = s; }
	bool HasShader() const { if (shader != nullptr) { return true; } else { return false; } }

protected:
	SceneNode* parent;
	Mesh* mesh;
	float distanceFromCamera;
	float boundingRadius;
	GLuint texture;
	GLuint bumpTexture;
	Matrix4 worldTransform;
	Matrix4 transform;
	Vector3 modelScale;
	Vector4 colour;
	std::vector<SceneNode*> children;

	//Added
	MeshMaterial* mat;
	vector<GLuint> textures;
	Shader* shader;
	GLuint metalnessTexture;
};