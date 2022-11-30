#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Camera.h"
#include "../nclgl/Frustum.h"
#include <algorithm>

class Light;
class Shader;
class MeshMaterial;
class MeshAnimation;
class SceneNode;

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
	void LoadTextures(); // Loads Textures
	void LoadShaders(); // Loads Shaders
	void LoadPostProcessing(); // Sets up openGL settings

	void LoadAssets(); // Calls functions to load assets
	void LoadMage(); // Load and position Mage asset
	void LoadGolem();
	void LoadSceneGraph();
	void LoadCameraPoints();
	SceneNode* LoadRock(Mesh* mesh, Vector4 colour, Vector3 scale, Vector3 pos, GLuint tex, Shader* shader);

	void PresentScene();
	void DrawPostProcess();
	void DrawScene();
	void DrawSkybox();
	void DrawHeightMap();
	void DrawWater();
	void DrawMage();
	void DrawGolem();

	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);

	void SwitchToPerspective();
	void SwitchToOrthographic();

	Shader* skyboxShader;
	Shader* reflectShader;
	Shader* sceneShader;
	Shader* processShader;
	Shader* bumpShader;
	Shader* mageShader;
	Shader* glassShader;
	Shader* golemShader;

	Camera* camera;

	Light* light;

	Frustum frameFrustum;

	SceneNode* root;
	vector <SceneNode*> transparentNodeList;
	vector <SceneNode*> nodeList;
	SceneNode* camPoints[5];

	//Mesh* quad;
	Mesh* waterQuad;
	HeightMap* hm;

	GLuint cubeMap;
	GLuint mapTex;
	GLuint mapBump;
	GLuint waterTex;

	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	GLuint cracksTex;
	GLuint poisonTex;
	GLuint glassTex;
	GLuint cubeTex;

	Vector3 glassParentPos;
	Mesh* glassQuad;
	Mesh* rock1;
	Mesh* rock2;
	Mesh* rock3;
	Mesh* rock4;
	Mesh* rock5;
	Mesh* cube;
	Mesh* quad;

	// Mage Vars
	Mesh* mageMesh;
	MeshAnimation* mageAnim;
	MeshMaterial* mageMat;
	vector<GLuint> mageTextures;
	int mageCurrentFrame;
	float mageFrameTime;

	//Golem Vars
	Mesh* golemMesh;
	MeshAnimation* golemAnim;
	MeshMaterial* golemMat;
	vector<GLuint> golemTextures;
	int golemCurrentFrame;
	float golemFrameTime;

	float waterRotate;
	float waterCycle;
	float timer;
	int camNo;
	int locked;
	int isPerspective;
	int isPostProcessing;
};