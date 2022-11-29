#include "renderer.h"
#include "renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/SceneNode.h"

const int POST_PASSES = 10;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	locked = 0;
	timer = 20;
	camNo = 0;
	
	isPostProcessing = 0;

	waterRotate = 0.0f;
	waterCycle = 0.0f;

	isPerspective = 1;
	SwitchToPerspective();

	camera = new Camera(-25.0f, 225.0f, Vector3(-150.0f, 250.0f, -150.0f));

	quad = Mesh::GenerateQuad();
	waterQuad = Mesh::GenerateQuad();

	hm = new HeightMap(CTEXTUREDIR"hm.png");
	
	LoadTextures();
	LoadShaders();

	if (!processShader->LoadSuccess() || !sceneShader->LoadSuccess())
		return;

	LoadPostProcessing();

	init = true;
}

Renderer::~Renderer(void) {
	delete skyboxShader;
	delete sceneShader;
	delete processShader;
	delete reflectShader;

	delete hm;
	delete quad;
	delete camera;

	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
}

void Renderer::LoadTextures() {
	heightTexture = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	//Night Galaxy
	cubeMap = SOIL_load_OGL_cubemap(
		CTEXTUREDIR"Left.jpg",
		CTEXTUREDIR"Right.jpg",
		CTEXTUREDIR"Up.jpg",
		CTEXTUREDIR"Down.jpg",
		CTEXTUREDIR"Front.jpg",
		CTEXTUREDIR"Back.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!heightTexture || !cubeMap || !waterTex)
		return;

	SetTextureRepeating(heightTexture, true);
}

void Renderer::LoadShaders() {
	sceneShader = new Shader("TexturedVertex.glsl", "texturedFragment.glsl");
	processShader = new Shader("TexturedVertex.glsl", "processFrag.glsl");
	skyboxShader = new Shader("SkyboxVertex.glsl", "skyboxFragment.glsl");
	reflectShader = new Shader("ReflectVertex.glsl", "reflectFragment.glsl");
}

void Renderer::LoadPostProcessing() {
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	for (int i = 0; i < 2; ++i) {
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0])
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt * 20);
	viewMatrix = camera->BuildViewMatrix();

	//Water cycle
	waterRotate += dt * 1.0f;
	waterCycle += dt * 0.25f;

}

void Renderer::RenderScene() {
	DrawScene();
	DrawPostProcess();
	PresentScene();
}

void Renderer::DrawScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	DrawSkybox();
	DrawWater();
	DrawHeightMap();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightMap() {
	BindShader(sceneShader);
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	UpdateShaderMatrices();
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightTexture);
	hm->Draw();
}

void Renderer::DrawWater() {
	BindShader(reflectShader);
	
	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);
	
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = hm->GetHeightmapSize();

	modelMatrix = Matrix4::Translation(hSize * 0.5f) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(-90, Vector3(1, 0, 0));
	modelMatrix = modelMatrix * Matrix4::Translation(Vector3(0, 0, -0.75));
	
	//textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();

	waterQuad->Draw();
}

void Renderer::DrawPostProcess() {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);
	for (int i = 0; i < POST_PASSES; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);

		glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
		quad->Draw();

		//SWAP

		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
		quad->Draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::PresentScene() {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(sceneShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	quad->Draw();
}

#pragma region Switch Perspectives

void Renderer::SwitchToPerspective() {
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
}

void Renderer::SwitchToOrthographic() {
	projMatrix = Matrix4::Orthographic(-1.0f, 10000.0f, width / 2.0f, -width / 2.0f, height / 2.0f, -height / 2.0f);
}

#pragma endregion

#pragma region Toggle Options

void Renderer::ToggleAutoCam() {
	locked >= 1 ? locked = 0 : locked = 1;
}

void Renderer::TogglePostProcessing() {
	std::cout << isPostProcessing << std::endl;
	isPostProcessing >= 1 ? isPostProcessing = 0 : isPostProcessing = 1;
	std::cout << isPostProcessing << std::endl;
}

void Renderer::ToggleCamPerspective() {
	isPerspective >= 1 ? isPerspective = 0 : isPerspective = 1;
}

#pragma endregion