#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Frustum.h"
#include <algorithm>

class HeightMap;
class Camera;
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

protected:
	void SetUpTex();
	void SpawnMage();
	//void Spawnskell();
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawMage();
	//void DrawObjs();
	//void Drawskell();
	void LoadCameraPoints(Vector3 orthPoint);
	void SpawnObjs();
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);
	void LoadScene();
	void LoadPostProcessing();
	void DrawPostProcessing();
	void ShowScene();
	void SwitchToPerspective();
	void SwitchToOrthographic();
	void TogglePostProcessing();
	void DrawScene();
	void ToggleCamPerspective();

	float camTimer;
	int locked;
	int isPerspective;
	int isPostProcessing;

	HeightMap* hm;
	Light* light;
	//Light* smallLight;
	Camera* camera;

	float waterRotate;
	float waterCycle;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint cracksTex;
	GLuint poisonTex;
	GLuint glassTex;

	Frustum frameFrustum;

	SceneNode* root;
	vector <SceneNode*> transparentNodeList;
	vector <SceneNode*> nodeList;
	SceneNode* camPoints[5];

	Mesh* glassQuad;
	Mesh* rock1;
	Mesh* rock2;
	Mesh* rock3;
	Mesh* rock4;
	Mesh* rock5;
	Mesh* cube;
	GLuint cubeTex;
	Mesh* quad;
	Mesh* postQuad;


	//Shader* glowShader;
	Shader* glassShader;
	Shader* bumpShader;
	Shader* reflectShader;
	Shader* skyboxShader;

	//Mage 
	Mesh* mage;
	MeshAnimation* mageAnim;
	MeshMaterial* mageMat;
	Shader* mageShader;
	vector<GLuint> mageTextures;
	int mageCurrentFrame;
	float mageFrameTime;

	//skell
	//Mesh* skell;
	//MeshAnimation* skellAnim;
	//MeshMaterial* skellMat;
	//Shader* skellShader;
	//vector<GLuint> skellTextures;
	//int skellCurrentFrame;
	//float skellFrameTime;

	//SceneNode* snowMan;
	//Shader* snowManShader;

	Shader* processShader;
	GLuint heightTexture;
	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	float timer;
	int camNo;
};