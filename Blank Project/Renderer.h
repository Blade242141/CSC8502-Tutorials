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

	SceneNode* camPoints[5];
	float camTimer;
	int locked;

	Shader* bumpShader;
	Shader* reflectShader;
	Shader* skyboxShader;

	HeightMap* hm;
	Mesh* quad;

	Light* light;
	Camera* camera;
	//Light* smallLight;

	void SwitchToPerspective();
	void SwitchToOrthographic();

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	

	float waterRotate;
	float waterCycle;

	void SpawnObjs();
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);

	SceneNode* root;
	Mesh* glassQuad;
	Shader* glassShader;
	GLuint glassTex;

	//SceneNode* snowMan;
	//Shader* snowManShader;

	Mesh* cube;
	//Shader* glowShader;
	GLuint cubeTex;

	void PrepHDRTex();
	Frustum frameFrustum;

	vector <SceneNode*> transparentNodeList;
	vector <SceneNode*> nodeList;

	//Mage 
	Mesh* mage;
	MeshAnimation* mageAnim;
	MeshMaterial* mageMat;
	Shader* mageShader;
	vector<GLuint> mageTextures;
	int mageCurrentFrame;
	float mageFrameTime;

	void LoadScene();
	Mesh* rock1;
	Mesh* rock2;
	Mesh* rock3;
	Mesh* rock4;
	Mesh* rock5;
	GLuint cracksTex;
	GLuint poisonTex;

	//skell
	//Mesh* skell;
	//MeshAnimation* skellAnim;
	//MeshMaterial* skellMat;
	//Shader* skellShader;
	//vector<GLuint> skellTextures;
	//int skellCurrentFrame;
	//float skellFrameTime;
};