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
	isPerspective = 1;
	SwitchToPerspective();

	quad = Mesh::GenerateQuad();
	hm = new HeightMap(CTEXTUREDIR"hm.png");

	SetUpTex();

	sceneShader = new Shader("TexturedVertex.glsl", "texturedFragment.glsl");
	processShader = new Shader("TexturedVertex.glsl", "processfrag.glsl");

	if (!sceneShader->LoadSuccess() || !processShader->LoadSuccess())
		return;

	Vector3 heightmapSize = hm->GetHeightmapSize();
	camera = new Camera(0.0f, 0.0f, Vector3(6992, 190, 6996));//heightmapSize * Vector3(0.5f, 5.0f, 0.5f));
	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 10), heightmapSize.x * 1.5);

	LoadPostProcessing();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterRotate = 0.0f;
	waterCycle = 0.0f;

	glEnable(GL_CULL_FACE);

	init = true;
}

Renderer::~Renderer(void) {
	delete camera;
	delete hm;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete bumpShader;
	delete light;

	delete bumpShader;

	delete root;
	delete glassQuad;
	delete glassShader;

	delete mage;
	delete mageAnim;
	delete mageMat;
	delete mageShader;

	//delete skell;
	//delete skellAnim;
	//delete skellMat;
	//delete skellShader;

	//delete snowMan;
	//delete snowManShader;

	delete rock1;
	delete rock2;
	delete rock3;
	delete rock4;
	delete rock5;
}

void Renderer::SetUpTex() {
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	glassTex = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cracksTex = SOIL_load_OGL_texture(CTEXTUREDIR"Cave_Cracks.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	poisonTex = SOIL_load_OGL_texture(CTEXTUREDIR"Cave_Poison.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//snowManTex = SOIL_load_OGL_texture(CTEXTUREDIR"Base_01.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//snowManBump = SOIL_load_OGL_texture(CTEXTUREDIR"Snow Man_01_Body_Normal.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//snowManSmoothness = SOIL_load_OGL_texture(CTEXTUREDIR"Snow Man_01_Body_MetallicSmoothness.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//glowTex = SOIL_load_OGL_texture(TEXTUREDIR"purple.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	//Night Galaxy
	cubeMap = SOIL_load_OGL_cubemap(
		CTEXTUREDIR"Left.jpg",
		CTEXTUREDIR"Right.jpg",
		CTEXTUREDIR"Up.jpg",
		CTEXTUREDIR"Down.jpg",
		CTEXTUREDIR"Front.jpg",
		CTEXTUREDIR"Back.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	//Day Little Cloudy
	//cubeMap = SOIL_load_OGL_cubemap(
	//	CTEXTUREDIR"MLeft.png",
	//	CTEXTUREDIR"MRight.png",
	//	CTEXTUREDIR"MUp.png",
	//	CTEXTUREDIR"MDown.png",
	//	CTEXTUREDIR"MFront.png",
	//	CTEXTUREDIR"MBack.png",
	//	SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!earthTex || !earthBump || !cubeMap || !waterTex || !glassTex || !cracksTex || !poisonTex)
		return;

	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
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

	glGenFramebuffers(1, &bufferFBO); //We’ll render the scene into this
	glGenFramebuffers(1, &processFBO);//And do post processing in this

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0]) {
		std::cout << "Error, FBO attachment was not successful!" << std::endl;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glEnable(GL_DEPTH_TEST);
}

void Renderer::DrawHeightmap() {
	BindShader(sceneShader);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	hm->Draw();
}

void Renderer::DrawScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	UpdateShaderMatrices();
	DrawHeightmap();

	SwitchToPerspective();
	UpdateShaderMatrices();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawPostProcessing() {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // This cuasing recompile

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
		//Now to swap the colour buffers , and do the second blur pass
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
		quad->Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::ShowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(sceneShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(glassShader->GetProgram(), "diffuseTex"), 0);
	quad->Draw();
}


void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt * 20);
	viewMatrix = camera->BuildViewMatrix();

}

void Renderer::RenderScene() {
	DrawScene();
	DrawPostProcessing();
	ShowScene();

}

#pragma region Switch Perspectives

void Renderer::SwitchToPerspective() {
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
}
