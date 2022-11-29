#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Camera.h"

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;
	void ToggleAutoCam();
	void ToggleCamPerspective();
	void TogglePostProcessing();

protected:
	void LoadTextures();
	void LoadShaders();
	void LoadPostProcessing();

	void PresentScene();
	void DrawPostProcess();
	void DrawScene();
	void DrawSkybox();
	void DrawHeightMap();
	void DrawWater();

	void SwitchToPerspective();
	void SwitchToOrthographic();

	Shader* skyboxShader;
	Shader* reflectShader;
	Shader* sceneShader;
	Shader* processShader;

	Camera* camera;

	Mesh* quad;
	Mesh* waterQuad;
	HeightMap* hm;

	GLuint cubeMap;
	GLuint heightTexture;
	GLuint waterTex;

	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	float waterRotate;
	float waterCycle;
	float timer;
	int camNo;
	int locked;
	int isPerspective;
	int isPostProcessing;
};