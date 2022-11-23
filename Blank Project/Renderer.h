#pragma once
#include "../NCLGL/OGLRenderer.h"
class HeightMap;
class Camera;
class Mesh;
class MeshAnimation;
class MeshMaterial;
class Light;
class HeightMap;

class Renderer : public OGLRenderer	{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	Camera* camera;
	Mesh* mesh;
	HeightMap* hm;
	GLuint hmTex;
	Shader* shader;
	MeshAnimation* anim;
	MeshMaterial* material;
	vector<GLuint> matTextures;
	Light* light;
	int currentFrame;
	float frameTime;
};
