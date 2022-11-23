#pragma once

#include "../nclgl/OGLRenderer.h"

class HeightMap;
class Camera;
class Light;
class Shader;
class MeshMaterial;
class MeshAnimation;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void SetUpTex();
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;

	HeightMap* hm;
	Mesh* quad;

	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;

	float waterRotate;
	float waterCycle;

	Mesh* dogPerson;
	Shader* dogShader;
	MeshAnimation* anim;
	MeshMaterial* mat;
	vector<GLuint> matTextures;
	int currentFrame;
	float frameTime;
};