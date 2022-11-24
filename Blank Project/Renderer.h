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

protected:
	void SetUpTex();
	void SpawnMage();
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawMage();
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

	void SpawnObjs();
	void BuildNodeLists(SceneNode * from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode * n);
	
	SceneNode* root;
	Mesh* glassQuad;
	Shader* glassShader;
	GLuint glassTex;
	
	Frustum frameFrustum;
	
	vector <SceneNode*> transparentNodeList;
	vector <SceneNode*> nodeList;

	//Mage 
	//Mesh* mageMesh;
	SceneNode* mage;
	MeshAnimation* mageAnim;

	MeshMaterial* mageMat;
	Shader* mageShader;
	vector<GLuint> mageTextures;
	int currentFrame;
	float frameTime;
	};