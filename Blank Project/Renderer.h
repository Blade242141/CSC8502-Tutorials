#pragma once
#include "../NCLGL/OGLRenderer.h"
class HeightMap;
class Camera;

class Renderer : public OGLRenderer	{
public:
	Renderer(Window &parent);
	 ~Renderer(void);
	 void RenderScene()				override;
	 void UpdateScene(float msec)	override;
	 void LoadScene();
protected:
	HeightMap* hm;
	Camera* cam;
	Shader* shader;
	GLuint hmTex;
};
