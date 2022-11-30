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
	isBEV = false;
	waterRotate = 0.0f;
	waterCycle = 0.0f;

	isPerspective = 1;
	SwitchToPerspective();

	camera = new Camera(0.0f, 225.0f, Vector3(6992, 200, 6996));
	quad = Mesh::GenerateQuad();
	waterQuad = Mesh::GenerateQuad();
	glassQuad = Mesh::GenerateQuad();

	hm = new HeightMap(CTEXTUREDIR"hm.png");
	LoadTextures();
	LoadShaders();

	if (!processShader->LoadSuccess() || !sceneShader->LoadSuccess())// || !barrelShader->LoadSuccess())
		return;

	Vector3 heightmapSize = hm->GetHeightmapSize();

	bev = new Camera(-90.0f, 0, Vector3(heightmapSize.x * 0.6, heightmapSize.y * 20, heightmapSize.z * 0.5));

	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1.5, 1.5, 1.5, 10), heightmapSize.x * 1.5);

	LoadAssets();

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
	mapTex = SOIL_load_OGL_texture(CTEXTUREDIR"Dirt.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	mapBump = SOIL_load_OGL_texture(CTEXTUREDIR"Dirt_Normal.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	
	glassTex = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cracksTex = SOIL_load_OGL_texture(CTEXTUREDIR"Cave_Cracks.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	poisonTex = SOIL_load_OGL_texture(CTEXTUREDIR"Cave_Poison.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	//barrelBase = SOIL_load_OGL_texture(CTEXTUREDIR"barrelBase.tif", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//barrelNormal = SOIL_load_OGL_texture(CTEXTUREDIR"barrelNormal.tif", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//barrelMetal = SOIL_load_OGL_texture(CTEXTUREDIR"barrelMetal.tif", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	//Night Galaxy
	cubeMap = SOIL_load_OGL_cubemap(
		CTEXTUREDIR"Left.jpg",
		CTEXTUREDIR"Right.jpg",
		CTEXTUREDIR"Up.jpg",
		CTEXTUREDIR"Down.jpg",
		CTEXTUREDIR"Front.jpg",
		CTEXTUREDIR"Back.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!mapTex || !cubeMap || !waterTex || !mapBump || !glassTex || !cracksTex || !poisonTex)// || !barrelBase || !barrelNormal || !barrelMetal)
		return;

	SetTextureRepeating(mapTex, true);
	SetTextureRepeating(mapBump, true);
	SetTextureRepeating(waterTex, true);
}

void Renderer::LoadShaders() {
	sceneShader = new Shader("TexturedVertex.glsl", "texturedFragment.glsl");
	processShader = new Shader("TexturedVertex.glsl", "processFrag.glsl");
	skyboxShader = new Shader("SkyboxVertex.glsl", "skyboxFragment.glsl");
	reflectShader = new Shader("ReflectVertex.glsl", "reflectFragment.glsl");
	bumpShader = new Shader("BumpVertex.glsl", "bumpFragment.glsl");
	mageShader = new Shader("SkinningVertex.glsl", "texturedFragment.glsl");
	glassShader = new Shader("SceneVertex.glsl", "sceneFragment.glsl");
	//barrelShader = new Shader("BarrelVertex.glsl", "barrelFrag.glsl");
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

}

void Renderer::LoadAssets() {
	LoadMage();
	//LoadGolem(); // Mesh wasnt loading correctly
	LoadSceneGraph();
	LoadCameraPoints();
}

void Renderer::LoadMage() {
	mageMesh = Mesh::LoadFromMeshFile("Coursework/Mage/Mage.msh");
	mageAnim = new MeshAnimation("Coursework/Mage/anim.anm");
	mageMat = new MeshMaterial("Coursework/Mage/Mage.mat");

	for (int i = 0; i < mageMesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = mageMat->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		mageTextures.emplace_back(texID);
	}

	mageCurrentFrame = 0;
	mageFrameTime = 0.0f;
}

void Renderer::LoadGolem() {
	golemMesh = Mesh::LoadFromMeshFile("Coursework/Golem/PBR_Golem.msh");
	golemAnim = new MeshAnimation("Coursework/Golem/anim.anm");
	golemMat = new MeshMaterial("Coursework/Golem/Golem.mat");

	for (int i = 0; i < golemMesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = golemMat->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		golemTextures.emplace_back(texID);
	}
	golemCurrentFrame = 0;
	golemFrameTime = 0.0f;
}

void Renderer::LoadSceneGraph() {
	Vector3 hmSize = hm->GetHeightmapSize();

	root = new SceneNode();

	SceneNode* rockParent = new SceneNode();
	glassParentPos = Vector3((hmSize.x * 0.5) - 100, 250, (hmSize.z * 0.5) - 100);
	rockParent->SetMesh(glassQuad);
	rockParent->SetTexture(glassTex);
	rockParent->SetColour(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
	rockParent->SetTransform(Matrix4::Translation(glassParentPos));
	rockParent->SetTransform(rockParent->GetTransform() * Matrix4::Rotation(180, Vector3(0, 1, 0)));
	rockParent->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
	rockParent->SetBoundingRadius(1.0f);

	rock1 = Mesh::LoadFromMeshFile("Coursework/Rocks/Rock1.msh");
	rock2 = Mesh::LoadFromMeshFile("Coursework/Rocks/Rock2.msh");
	rock3 = Mesh::LoadFromMeshFile("Coursework/Rocks/Rock3.msh");
	rock4 = Mesh::LoadFromMeshFile("Coursework/Rocks/Rock4.msh");
	rock5 = Mesh::LoadFromMeshFile("Coursework/Rocks/Rock5.msh");
	//barrel = Mesh::LoadFromMeshFile("Coursework/barrel.msh");
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh");

	SceneNode* refletCube = new SceneNode();
	refletCube->SetMesh(cube);
	refletCube->SetColour(Vector4(0.5f, 0.5f, 0.5f, 1));
	refletCube->SetShader(reflectShader);
	refletCube->SetTransform(Matrix4::Translation(Vector3(0, 0, 0)));
	refletCube->SetBoundingRadius(1);
	refletCube->SetModelScale(Vector3(25, 25, 25));

	//SceneNode* barrelObj = new SceneNode();
	//barrelObj->SetMesh(barrel);
	//barrelObj->SetShader(barrelShader);
	//barrelObj->SetTexture(barrelBase);
	//barrelObj->SetBumpTxtures(barrelNormal);
	//barrelObj->SetMetalTexture(barrelMetal);
	//barrelObj->SetTransform(Matrix4::Translation(Vector3(10, -5, 10)));
	//barrelObj->SetModelScale(Vector3(10, 10, 10));

	root->AddChild(rockParent);
	//rockParent->AddChild(barrelObj);

	rockParent->AddChild(LoadRock(rock1, Vector4(5, 225, 0, 0.98f), Vector3(100, 100, 100), Vector3(250, -100, 250), cracksTex, 0, bumpShader));
	rockParent->AddChild(LoadRock(rock2, Vector4(1, 1, 1, 1), Vector3(100, 100, 100), Vector3(500, -100, 0), cracksTex, 0, bumpShader));
	rockParent->AddChild(LoadRock(rock3, Vector4(1, 1, 1, 1), Vector3(100, 100, 100), Vector3(0, -100, 500), poisonTex, 0, bumpShader));
	rockParent->AddChild(LoadRock(rock4, Vector4(1, 1, 1, 1), Vector3(100, 100, 100), Vector3(-250, -100, 250), cracksTex, 0, bumpShader));
	rockParent->AddChild(LoadRock(rock5, Vector4(1, 1, 1, 1), Vector3(100, 100, 100), Vector3(250, -150, -250), poisonTex, 0, bumpShader));
	rockParent->AddChild(LoadRock(rock3, Vector4(1, 1, 1, 1), Vector3(100, 100, 100), Vector3(-500, -100, 0), cracksTex, 0, bumpShader));
	rockParent->AddChild(LoadRock(rock4, Vector4(1, 1, 1, 1), Vector3(100, 100, 100), Vector3(0, -100, -500), poisonTex, 0, bumpShader));
	rockParent->AddChild(refletCube);

	//smallLight = new Light(Vector3((hmSize.x * 0.5) - 200, 250, (hmSize.z * 0.5) - 200), Vector4(70, 36, 183, 1), 50.0f);
}

SceneNode* Renderer::LoadRock(Mesh* mesh, Vector4 colour, Vector3 scale, Vector3 pos, GLuint tex, GLuint bump, Shader* shader) {
	SceneNode* s = new SceneNode();
	s->SetMesh(mesh);
	s->SetColour(colour);
	s->SetModelScale(scale);
	s->SetTransform(Matrix4::Translation(pos));
	s->SetBoundingRadius(1);
	s->SetTexture(tex);
	s->SetShader(shader);
	s->SetBumpTxtures(bump);
	return s;
}

void Renderer::LoadCameraPoints() {
	SceneNode* cameraPointParent = new SceneNode();
	SceneNode* point1 = new SceneNode();
	point1->SetTransform(Matrix4::Translation(Vector3(7165.55f, 889.969f, 5530.38f)));
	camPoints[0] = point1;

	SceneNode* point2 = new SceneNode();
	point2->SetTransform(Matrix4::Translation(Vector3(9027.21f, 209.974f, 7878.02f)));
	camPoints[1] = point2;

	SceneNode* point3 = new SceneNode();
	point3->SetTransform(Matrix4::Translation(Vector3(7897.7f, 209.974f, 7770.87f)));
	camPoints[2] = point3;

	SceneNode* point4 = new SceneNode();
	point4->SetTransform(Matrix4::Translation(Vector3(7236.06f, 270.013f, 8187.66f)));
	camPoints[3] = point4;

	//SceneNode* point5 = new SceneNode();
	//point5->SetTransform(Matrix4::Translation(orthPoint));
	//camPoints[4] = point5;

	root->AddChild(cameraPointParent);
	cameraPointParent->AddChild(point1);
	cameraPointParent->AddChild(point2);
	cameraPointParent->AddChild(point3);
	cameraPointParent->AddChild(point4);
	//cameraPointParent->AddChild(point5);
}

void Renderer::UpdateScene(float dt) {
	timer -= dt;
	if (locked == 1) {

		if (timer <= 0) {
			camNo++;
			timer = 20;
		}

		switch (camNo) {
		case 0:
			camera->SetPosition(camPoints[0]->GetTransform().GetPositionVector());
			break;
		case 1:
			camera->SetPosition(camPoints[1]->GetTransform().GetPositionVector());
			break;
		case 2:
			camera->SetPosition(camPoints[2]->GetTransform().GetPositionVector());
			break;
		case 3:
			camera->SetPosition(camPoints[3]->GetTransform().GetPositionVector());
			break;
		case 4:
			camera->SetPosition(camPoints[4]->GetTransform().GetPositionVector());
			break;
		default:
			camNo = 0;
		}
	}

	camera->UpdateCamera(dt * 20, true);
	viewMatrix = camera->BuildViewMatrix();

	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	root->Update(dt);

	//Water cycle
	waterRotate += dt * 1.0f;
	waterCycle += dt * 0.25f;

	//Mage Animation
	mageFrameTime -= dt;
	while (mageFrameTime < 0.0f) {
		mageCurrentFrame = (mageCurrentFrame + 1) % mageAnim->GetFrameCount();
		mageFrameTime += 1.0f / mageAnim->GetFrameRate();
	}

	//Golem Animation
	//golemFrameTime -= dt;
	//while (golemFrameTime < 0.0f) {
	//	golemCurrentFrame = (golemCurrentFrame + 1) % golemAnim->GetFrameCount();
	//	golemFrameTime += 1.0f / golemAnim->GetFrameRate();
	//}
}

void Renderer::RightScene() {
	bev->UpdateCamera(0, false);
	viewMatrix = bev->BuildViewMatrix();
}

void Renderer::RenderScene() {

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	isBEV ? glViewport(0, 0, GetWidth() / 2, GetHeight()) : glViewport(0, 0, GetWidth(), GetHeight());

	DrawScene();
	if (isPostProcessing) {
		DrawPostProcess();
		PresentScene();
	}

	if (isBEV) {
		glViewport(GetWidth() / 2, 0, GetWidth(), GetHeight());
		RightScene();
		DrawScene();
	}
}

void Renderer::DrawScene() {
	if (isPostProcessing) { glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO); }
	BuildNodeLists(root);
	SortNodeLists();

	DrawSkybox();
	DrawMage();
	//DrawGolem(); // Mesh wasnt loading correcly
	DrawNodes();
	DrawHeightMap();
	if (!isPostProcessing) { DrawWater(); } // Post processing causes issues when adjusting texture matrix

	if (isPostProcessing) { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	ClearNodeLists();

}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightMap() {
	BindShader(bumpShader);
	isPerspective == 1? SwitchToPerspective() : SwitchToOrthographic();
	SetShaderLight(*light);
	UpdateShaderMatrices();

	glUniform3fv(glGetUniformLocation(bumpShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(bumpShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mapTex);

	glUniform1i(glGetUniformLocation(bumpShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mapBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

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

	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();

	waterQuad->Draw();
}

void Renderer::DrawMage() {
	BindShader(mageShader);
	glUniform1i(glGetUniformLocation(mageShader->GetProgram(), "diffuseTex"), 0);
	modelMatrix = Matrix4::Translation(glassParentPos) * Matrix4::Scale(Vector3(100, 100, 100)) * Matrix4::Rotation(180, Vector3(0, 1, 0));

	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;
	const Matrix4* invBindPose = mageMesh->GetInverseBindPose();
	const Matrix4* frameData = mageAnim->GetJointData(mageCurrentFrame);

	for (unsigned int i = 0; i < mageMesh->GetJointCount(); ++i)
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);

	int j = glGetUniformLocation(mageShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < mageMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mageTextures[i]);
		mageMesh->DrawSubMesh(i);
	}
}

void Renderer::DrawGolem() {
	BindShader(mageShader);
	glUniform1i(glGetUniformLocation(mageShader->GetProgram(), "diffuseTex"), 0);
	modelMatrix = Matrix4::Translation(Vector3(1, -15, 1)) * Matrix4::Scale(Vector3(0.1, 0.1, 0.1)) * Matrix4::Rotation(180, Vector3(0, 1, 0));
	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;
	const Matrix4* invBindPose = golemMesh->GetInverseBindPose();
	const Matrix4* frameData = golemAnim->GetJointData(golemCurrentFrame);

	for (unsigned int i = 0; i < golemMesh->GetJointCount(); ++i)
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);

	int j = glGetUniformLocation(mageShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());


	for (int i = 0; i < golemMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, golemTextures[i]);
		golemMesh->DrawSubMesh(i);
	}
}

void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}
	for (vector <SceneNode*>::const_iterator i = from->GetCHildIteratorStart(); i != from->GetChildIteratorEnd(); ++i)
		BuildNodeLists((*i));
}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::DrawNodes() {
	for (const auto& i : nodeList)
		DrawNode(i);

	for (const auto& i : transparentNodeList)
		DrawNode(i);
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Shader* shader;
		if (n->HasShader())
			shader = n->GetShader();
		else
			shader = glassShader;

		BindShader(shader);
		UpdateShaderMatrices();
		glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);

		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, model.values);

		glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

		glassTex = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glassTex);

		glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), glassTex);
		n->Draw(*this);
		//if (n->HasBump() && n->HasMetal()) {
		//	glUniform1i(glGetUniformLocation(shader->GetProgram(), "bumpTex"), 1);
		//	//glUniform1i(glGetUniformLocation(shader->GetProgram(), "metalnessTex"), 2);
		//	
		//	GLuint tex = n->GetTexture();

		//	glUniform3fv(glGetUniformLocation(shader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
		//	glActiveTexture(GL_TEXTURE0);
		//	glBindTexture(GL_TEXTURE_2D, n->GetTexture());

		//	glActiveTexture(GL_TEXTURE1);
		//	glBindTexture(GL_TEXTURE_2D, n->GetBumpTextures());

		//	//glActiveTexture(GL_TEXTURE2);
		//	//glBindTexture(GL_TEXTURE_2D, n->GetMetalTexture());

		//}
		//else {

		//}
	}
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

void Renderer::ToggleBEV() {
	isBEV >= 1 ? isBEV = 0 : isBEV = 1;
}

#pragma endregion