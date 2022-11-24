#include "renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/SceneNode.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {

	quad = Mesh::GenerateQuad();
	glassQuad = Mesh::GenerateQuad();

	hm = new HeightMap(CTEXTUREDIR"hm1.png");

	SetUpTex();

	reflectShader = new Shader("ReflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "perPixelFragment.glsl");
	glassShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	mageShader = new Shader("SkinningVertex.glsl", "texturedFragment.glsl");

	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !glassShader->LoadSuccess() || !mageShader->LoadSuccess())
		return;

	Vector3 heightmapSize = hm->GetHeightmapSize();
	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.5f, 5.0f, 0.5f));
	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 10), heightmapSize.x * 1.5);

	//Load Objects into Scene

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterRotate = 0.0f;
	waterCycle = 0.0f;

	glEnable(GL_CULL_FACE);
	SpawnObjs();

	init = true;
}

Renderer::~Renderer(void) {
	delete camera;
	delete hm;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete light;
}

void Renderer::SetUpTex() {
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	glassTex = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	//Night Galaxy
	//cubeMap = SOIL_load_OGL_cubemap(
	//	CTEXTUREDIR"Left.jpg",
	//	CTEXTUREDIR"Right.jpg",
	//	CTEXTUREDIR"Up.jpg",
	//	CTEXTUREDIR"Down.jpg",
	//	CTEXTUREDIR"Front.jpg",
	//	CTEXTUREDIR"Back.jpg",
	//	SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	//Day Little Cloudy
	cubeMap = SOIL_load_OGL_cubemap(
		CTEXTUREDIR"MLeft.png",
		CTEXTUREDIR"MRight.png",
		CTEXTUREDIR"MUp.png",
		CTEXTUREDIR"MDown.png",
		CTEXTUREDIR"MFront.png",
		CTEXTUREDIR"MBack.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!earthTex || !earthBump || !cubeMap || !waterTex || !glassTex)
		return;

	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
}

void Renderer::SpawnObjs() {
	root = new SceneNode();

	SceneNode* glass = new SceneNode();
	glass->SetColour(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
	glass->SetTransform(Matrix4::Translation(Vector3(0, 100.0f, -300.0f + 100.0f + 100)));
	glass->SetTransform(Matrix4::Rotation(180, Vector3(1, 0, 0)));
	glass->SetModelScale(Vector3(1000.0f, 1000.0f, 1000.0f));
	glass->SetBoundingRadius(100.0f);
	glass->SetMesh(glassQuad);
	glass->SetTexture(glassTex);

	root->AddChild(glass);
	SpawnMage();
}

void Renderer::SpawnMage() {
	mage = new SceneNode(Mesh::LoadFromMeshFile("Coursework/Golem/PBR_Golem.msh"));

	//mageMesh = Mesh::LoadFromMeshFile("Coursework/Golem/Golem.msh");
	mageAnim = new MeshAnimation("Coursework/Golem/anim1.anm");
	mageMat = new MeshMaterial("Coursework/Golem/Golem.mat");

	for (int i = 0; i < mage->GetMesh()->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = mageMat->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		std::cout << "filename = " << *filename << std::endl;
		string path = TEXTUREDIR + *filename;
		std::cout << "str = " << path << std::endl;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		mageTextures.emplace_back(texID);
	}


	//mage->SetTransform(Matrix4::Translation(Vector3(0, 100.0f, -300.0f + 100.0f + 100)));
	//mage->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));

	//mage->SetTransform(Matrix4::Rotation(90.0f, Vector3(1, 0, 0)));
	//mage->SetBoundingRadius(100.0f);

	frameTime = 0.0f;
	currentFrame = 0;
	root->AddChild(mage);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt * 20);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	root->Update(dt);

	//Water cycle
	waterRotate += dt * 2.0f;
	waterCycle += dt * 0.25f;

	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % mageAnim->GetFrameCount();
		frameTime += 1.0f / mageAnim->GetFrameRate();
	}
}

void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	DrawSkybox();
	DrawHeightmap();
	DrawMage();
	DrawWater();

	BindShader(glassShader);
	UpdateShaderMatrices();
	
	glUniform1i(glGetUniformLocation(glassShader->GetProgram(), "diffuseTex"), 0);
	DrawNodes();
	
	ClearNodeLists();
}

void Renderer::DrawMage() {
	BindShader(mageShader);
	glUniform1i(glGetUniformLocation(mageShader->GetProgram(), "diffuseTex"), 0);

	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;
	const Matrix4* invBindPose = mage->GetMesh()->GetInverseBindPose();
	const Matrix4* frameData = mageAnim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < mage->GetMesh()->GetJointCount(); ++i)
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);

	int j = glGetUniformLocation(mageShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size()*5, false, (float*)frameMatrices.data());

	for (int i = 0; i < mage->GetMesh()->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mageTextures[i]);
		mage->GetMesh()->DrawSubMesh(i);
	}
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	hm->Draw();
}

void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		} else {
			nodeList.push_back(from);
		}
	}
	for (vector <SceneNode*>::const_iterator i = from->GetCHildIteratorStart(); i != from->GetChildIteratorEnd(); ++i)
		BuildNodeLists((*i));
}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(),	transparentNodeList.rend(),	SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(),	nodeList.end(),	SceneNode::CompareByCameraDistance);
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
		Matrix4 model = n->GetWorldTransform() *
			Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(
			glGetUniformLocation(glassShader->GetProgram(), "modelMatrix"), 1, false, model.values);

		glUniform4fv(glGetUniformLocation(glassShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

		glassTex = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, glassTex);

		glUniform1i(glGetUniformLocation(glassShader->GetProgram(), "useTexture"), glassTex);
		n->Draw(*this);
	}
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = hm->GetHeightmapSize();

	modelMatrix = Matrix4::Translation(hSize * 0.5f) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(-90, Vector3(1, 0, 0));

	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));


	UpdateShaderMatrices();
	//SetShaderLight(*light);

	quad->Draw();
}