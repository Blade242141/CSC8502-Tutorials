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
	locked = false;
	quad = Mesh::GenerateQuad();
	glassQuad = Mesh::GenerateQuad();

	hm = new HeightMap(CTEXTUREDIR"hm.png");

	SetUpTex();
	mageShader = new Shader("SkinningVertex.glsl", "texturedFragment.glsl");
	reflectShader = new Shader("ReflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("SkyboxVertex.glsl", "skyboxFragment.glsl");
	bumpShader = new Shader("BumpVertex.glsl", "bumpFragment.glsl");
	glassShader = new Shader("SceneVertex.glsl", "sceneFragment.glsl");
	//snowManShader = new Shader("SkinningVertex.glsl", "texturedFragment.glsl");
	//skellShader = new Shader("SkinningVertex.glsl", "texturedFragment.glsl");
	//glowShader = new Shader("SceneVertex.glsl", "hdrFragment.glsl");

	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !bumpShader->LoadSuccess() || !glassShader->LoadSuccess())// || !mageShader->LoadSuccess()); // || !snowManShader->LoadSuccess()) //!skellShader->LoadSuccess())
		return;

	Vector3 heightmapSize = hm->GetHeightmapSize();
	camera = new Camera(0.0f, 0.0f, Vector3(6992, 190, 6996));//heightmapSize * Vector3(0.5f, 5.0f, 0.5f));
	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 10), heightmapSize.x * 1.5);
	//Load Objects into Scene



	SpawnObjs();
	SwitchToPerspective();

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

void Renderer::SpawnObjs() {
	root = new SceneNode();

	
	/*Spawn cube for HDR shader*/
	//cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh");
	//SceneNode* glowCube = new SceneNode(cube, Vector4(0.5, 0.5, 0.5, 1));
	//glowCube->SetModelScale(Vector3(25, 25, 25));
	//glowCube->SetBoundingRadius(100.0f);
	//glowCube->SetTransform(Matrix4::Translation(Vector3(50, 10, 50)));
	//glowCube->SetTexture(glowTex);
	//root->AddChild(glowCube);

	/*Attempt to spawn a humaniod mesh with material as an SceneNode*/
	//snowMan = new SceneNode();
	//snowMan->SetMesh(Mesh::LoadFromMeshFile("Coursework/SnowMan/snowman.msh"));
	//snowMan->SetTransform(Matrix4::Translation(Vector3(1, 1, 1)));
	//snowMan->SetBoundingRadius(100.0f);
	//snowMan->SetMeshMaterial(new MeshMaterial("Coursework/SnowMan/SnowMan.mat"));
	//for (int i = 0; i < snowMan->GetMesh()->GetSubMeshCount(); ++i) {
	//	const MeshMaterialEntry* matEntry = snowMan->GetMeshMaterial()->GetMaterialForLayer(i);
	//	const string* filename = nullptr;
	//	matEntry->GetEntry("Diffuse", &filename);
	//	string path = TEXTUREDIR + *filename;
	//	GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
	//	snowMan->GetTextures().emplace_back(texID);
	//}

	//root->AddChild(snowMan);

	LoadScene();
	SpawnMage();
	//Spawnskell();
}

void Renderer::LoadScene() {
	Vector3 hmSize = hm->GetHeightmapSize();

	SceneNode* rockParent = new SceneNode();
	rockParent->SetMesh(glassQuad);
	rockParent->SetTexture(glassTex);
	rockParent->SetColour(Vector4(1.0f, 1.0f, 1.0f, 0.5f));
	rockParent->SetTransform(Matrix4::Translation(Vector3((hmSize.x * 0.5) - 100, 250, (hmSize.z * 0.5) - 100)));
	rockParent->SetTransform(rockParent->GetTransform() * Matrix4::Rotation(180, Vector3(0, 1, 0)));
	rockParent->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));
	rockParent->SetBoundingRadius(1.0f);

	rock1 = Mesh::LoadFromMeshFile("Coursework/Rocks/Rock1.msh");
	rock2 = Mesh::LoadFromMeshFile("Coursework/Rocks/Rock2.msh");
	rock3 = Mesh::LoadFromMeshFile("Coursework/Rocks/Rock3.msh");
	rock4 = Mesh::LoadFromMeshFile("Coursework/Rocks/Rock4.msh");
	rock5 = Mesh::LoadFromMeshFile("Coursework/Rocks/Rock5.msh");
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh");

	SceneNode* refletCube = new SceneNode();
	refletCube->SetMesh(cube);
	refletCube->SetColour(Vector4(0.5f, 0.5f, 0.5f, 1));
	refletCube->SetShader(reflectShader);
	refletCube->SetTransform(Matrix4::Translation(Vector3(0, 0, 0)));
	refletCube->SetBoundingRadius(1);
	refletCube->SetModelScale(Vector3(25, 25, 25));
	//refletCube->SetTexture(cubeTex);

	SceneNode* rockA = new SceneNode();
	rockA->SetMesh(rock1);
	rockA->SetColour(Vector4(1, 1, 1, 1));
	rockA->SetModelScale(Vector3(100, 100, 100));
	rockA->SetTransform(Matrix4::Translation(Vector3(250, -100, 250)));
	rockA->SetBoundingRadius(1);
	rockA->SetTexture(poisonTex);
	rockA->SetShader(bumpShader);

	SceneNode* rockB = new SceneNode();
	rockB->SetMesh(rock2);
	rockB->SetColour(Vector4(1, 1, 1, 1));
	rockB->SetModelScale(Vector3(100, 100, 100));
	rockB->SetTransform(Matrix4::Translation(Vector3(500, -100, 0)));
	rockB->SetBoundingRadius(1);
	rockB->SetTexture(cracksTex);
	rockB->SetShader(bumpShader);

	SceneNode* rockC = new SceneNode();
	rockC->SetMesh(rock3);
	rockC->SetColour(Vector4(1, 1, 1, 1));
	rockC->SetModelScale(Vector3(100, 100, 100));
	rockC->SetTransform(Matrix4::Translation(Vector3(0, -100, 500)));
	rockC->SetBoundingRadius(1);
	rockC->SetTexture(poisonTex);
	rockC->SetShader(bumpShader);

	SceneNode* rockD = new SceneNode();
	rockD->SetMesh(rock4);
	rockD->SetColour(Vector4(1, 1, 1, 1));
	rockD->SetModelScale(Vector3(100, 100, 100));
	rockD->SetTransform(Matrix4::Translation(Vector3(-250, -100, 250)));
	rockD->SetBoundingRadius(1);
	rockD->SetTexture(cracksTex);
	rockD->SetShader(bumpShader);

	SceneNode* rockE = new SceneNode();
	rockE->SetMesh(rock5);
	rockE->SetColour(Vector4(1, 1, 1, 1));
	rockE->SetModelScale(Vector3(100, 100, 100));
	rockE->SetTransform(Matrix4::Translation(Vector3(250, -150, -250)));
	rockE->SetBoundingRadius(100);
	rockE->SetTexture(poisonTex);
	rockE->SetShader(bumpShader);

	SceneNode* rockF = new SceneNode();
	rockF->SetMesh(rock3);
	rockF->SetColour(Vector4(1, 1, 1, 1));
	rockF->SetModelScale(Vector3(100, 100, 100));
	rockF->SetTransform(Matrix4::Translation(Vector3(-500, -100, 0)));
	rockF->SetBoundingRadius(1);
	rockF->SetTexture(cracksTex);
	rockF->SetShader(bumpShader);

	SceneNode* rockG = new SceneNode();
	rockG->SetMesh(rock4);
	rockG->SetColour(Vector4(1, 1, 1, 1));
	rockG->SetModelScale(Vector3(100, 100, 100));
	rockG->SetTransform(Matrix4::Translation(Vector3(0, -100, -500)));
	rockG->SetBoundingRadius(1);
	rockG->SetTexture(poisonTex);
	rockG->SetShader(bumpShader);

	//smallLight = new Light(Vector3((hmSize.x * 0.5) - 200, 250, (hmSize.z * 0.5) - 200), Vector4(70, 36, 183, 1), 50.0f);

	root->AddChild(rockParent);
	rockParent->AddChild(rockA);
	rockParent->AddChild(rockB);
	rockParent->AddChild(rockC);
	rockParent->AddChild(rockD);
	rockParent->AddChild(rockE);
	rockParent->AddChild(rockF);
	rockParent->AddChild(rockG);
	rockParent->AddChild(refletCube);
	LoadCameraPoints(refletCube->GetTransform().GetPositionVector());
}

void Renderer::LoadCameraPoints(Vector3 orthPoint) {
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

	SceneNode* point5 = new SceneNode();
	point5->SetTransform(Matrix4::Translation(orthPoint));
	camPoints[4] = point5;

	root->AddChild(cameraPointParent);
	cameraPointParent->AddChild(point1);
	cameraPointParent->AddChild(point2);
	cameraPointParent->AddChild(point3);
	cameraPointParent->AddChild(point4);
	cameraPointParent->AddChild(point5);

}

void Renderer::SpawnMage() {
	//mage = new SceneNode(Mesh::LoadFromMeshFile("Coursework/Mage/Mage.msh"));

	mage = Mesh::LoadFromMeshFile("Coursework/Mage/Mage.msh");
	mageAnim = new MeshAnimation("Coursework/Mage/anim.anm");
	mageMat = new MeshMaterial("Coursework/Mage/Mage.mat");

	for (int i = 0; i < mage->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = mageMat->GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		mageTextures.emplace_back(texID);
	}

	mageCurrentFrame = 0;
	mageFrameTime = 0.0f;
	//mage->SetTransform(Matrix4::Translation(Vector3(0, 100.0f, -300.0f + 100.0f + 100)));
	//mage->SetModelScale(Vector3(100.0f, 100.0f, 100.0f));

	//mage->SetTransform(Matrix4::Rotation(90.0f, Vector3(1, 0, 0)));
	//mage->SetBoundingRadius(100.0f);
	//root->AddChild(mage);
}

//void Renderer::Spawnskell() {
//	skell = Mesh::LoadFromMeshFile("Coursework/skell/skell.msh");
//	skellAnim = new MeshAnimation("Coursework/skell/anim.anm");
//	skellMat = new MeshMaterial("Coursework/skell/skell.mat");
//
//	for (int i = 0; i < skell->GetSubMeshCount(); ++i) {
//		const MeshMaterialEntry* matEntry = skellMat->GetMaterialForLayer(i);
//		const string* filename = nullptr;
//		matEntry->GetEntry("Diffuse", &filename);
//		string path = TEXTUREDIR + *filename;
//		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
//		skellTextures.emplace_back(texID);
//	}
//	skellCurrentFrame = 0;
//	skellFrameTime = 0.0f;
//}

float timer = 20;
int camNo = 0;
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
			SwitchToPerspective();
			break;
		case 1:
			camera->SetPosition(camPoints[1]->GetTransform().GetPositionVector());
			SwitchToPerspective();
			break;
		case 2:
			camera->SetPosition(camPoints[2]->GetTransform().GetPositionVector());
			SwitchToPerspective();
			break;
		case 3:
			camera->SetPosition(camPoints[3]->GetTransform().GetPositionVector());
			SwitchToPerspective();
			break;
		case 4:
			camera->SetPosition(camPoints[4]->GetTransform().GetPositionVector());
			SwitchToOrthographic();
			break;
		default:
			camNo = 0;
		}
	}

	camera->UpdateCamera(dt * 20);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	root->Update(dt);
	
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_L))
		std::cout << "x = " << camera->GetPosition().x << ", y = " << camera->GetPosition().y << ", z = " << camera->GetPosition().z << std::endl;


	//Water cycle
	waterRotate += dt * 1.0f;
	waterCycle += dt * 0.25f;

	mageFrameTime -= dt;
	//skellFrameTime -= dt;
	while (mageFrameTime < 0.0f) {
		mageCurrentFrame = (mageCurrentFrame + 1) % mageAnim->GetFrameCount();
		mageFrameTime += 1.0f / mageAnim->GetFrameRate();
	}
	//while (skellFrameTime < 0.0f) {
	//	skellCurrentFrame = (skellCurrentFrame + 1) % skellAnim->GetFrameCount();
	//	skellFrameTime += 1.0f / skellAnim->GetFrameRate();
	//}
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BuildNodeLists(root);
	SortNodeLists();
	PrepHDRTex();

	DrawSkybox();
	DrawHeightmap();
	//Drawskell();	//Mesh loads incorrectly, axes move but humanoid is not correct 
	DrawMage();
	DrawWater();
	//DrawObjs();	//Unable to get mesh to render on screen

	DrawNodes();

	ClearNodeLists();
}

void Renderer::PrepHDRTex() {
	//Removed code breaking the whole thing
	//Reattempt Later
}

void Renderer::DrawMage() {
	BindShader(mageShader);
	glUniform1i(glGetUniformLocation(mageShader->GetProgram(), "diffuseTex"), 0);

	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;
	const Matrix4* invBindPose = mage->GetInverseBindPose();
	const Matrix4* frameData = mageAnim->GetJointData(mageCurrentFrame);

	for (unsigned int i = 0; i < mage->GetJointCount(); ++i)
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);

	int j = glGetUniformLocation(mageShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	modelMatrix = Matrix4::Translation((camPoints[4]->GetTransform().GetPositionVector())) * Matrix4::Scale(Vector3(100, 100, 100)) * Matrix4::Rotation(180, Vector3(0, 1, 0));


	for (int i = 0; i < mage->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mageTextures[i]);
		mage->DrawSubMesh(i);
	}
}

//void Renderer::DrawObjs() {
//	BindShader(snowManShader);
//	glUniform1i(glGetUniformLocation(snowManShader->GetProgram(), "diffuseTex"), 0);
//	
//	UpdateShaderMatrices();
//
//	vector<Matrix4> frameMatrices;
//	const Matrix4* invBindPose = snowMan->GetMesh()->GetInverseBindPose();
//
//	for (unsigned int i = 0; i < snowMan->GetMesh()->GetJointCount(); ++i)
//		frameMatrices.emplace_back(invBindPose[i]);
//
//	int j = glGetUniformLocation(snowManShader->GetProgram(), "joints");
//	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());
//
//	for (int i = 0; i < mage->GetSubMeshCount(); ++i) {
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, mageTextures[i]);
//		snowMan->GetMesh()->DrawSubMesh(i);
//	}
//}

//void Renderer::Drawskell() {
//	BindShader(skellShader);
//	glUniform1i(glGetUniformLocation(skellShader->GetProgram(), "diffuseTex"), 0);
//
//	UpdateShaderMatrices();
//
//	vector<Matrix4> frameMatrices;
//	const Matrix4* invBindPose = skell->GetInverseBindPose();
//	const Matrix4* frameData = skellAnim->GetJointData(skellCurrentFrame);
//
//	for (unsigned int i = 0; i < skell->GetJointCount(); ++i)
//		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
//
//	int j = glGetUniformLocation(skellShader->GetProgram(), "joints");
//	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());
//
//	//modelMatrix = Matrix4::Translation(Vector3(1, -15, 1)) * Matrix4::Scale(Vector3(1, 1, 1)) * Matrix4::Rotation(180, Vector3(0, 1, 0));
//
//	for (int i = 0; i < skell->GetSubMeshCount(); ++i) {
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, skellTextures[i]);
//		skell->DrawSubMesh(i);
//	}
//}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	BindShader(bumpShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(bumpShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(bumpShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(bumpShader->GetProgram(), "bumpTex"), 1);
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
	modelMatrix = modelMatrix * Matrix4::Translation(Vector3(0, 0, -0.75));
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));


	UpdateShaderMatrices();
	//SetShaderLight(*light);

	quad->Draw();
}

void Renderer::SwitchToPerspective() {
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
}

void Renderer::SwitchToOrthographic() {
	projMatrix = Matrix4::Orthographic(-1.0f, 10000.0f, width / 2.0f, -width / 2.0f, height / 2.0f, -height / 2.0f);
}

void Renderer::ToggleAutoCam() {
	std::cout << locked << std::endl;
	locked >= 1 ? locked = 0 : locked = 1;
	std::cout << locked << std::endl;
}