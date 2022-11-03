#include "renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	meshes[0] = Mesh::GenerateQuad();
	meshes[1] = Mesh::GenerateTriangle();

	textures[0] = SOIL_load_OGL_texture(TEXTUREDIR"brick.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	textures[1] = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	if (!textures[0] || !textures[1])
		return;

	positions[0] = Vector3(0, 0, -5);
	positions[1] = Vector3(0, 0, -5);

	shader = new Shader("TexturedVertex.glsl", "texturedFragment.glsl");

	if (!shader->LoadSuccess())
		return;

	usingDepth = false;
	usingAlpha = false;
	blendMode = 0;
	modifyObject = true;

	projMatrix = Matrix4::Perspective(1.0f, 100.0f, (float)width / (float)height, 45.0f);

	init = true;
}

Renderer::~Renderer(void) {
	delete meshes[0];
	delete meshes[1];
	delete shader;
	glDeleteTextures(2, textures);
}

