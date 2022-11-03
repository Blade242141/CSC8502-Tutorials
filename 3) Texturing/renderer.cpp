#include "renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	triangle = Mesh::GenerateTriangle();

	texture = SOIL_load_OGL_texture(TEXTUREDIR"brick.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);

	if (!texture)
		return;

	shader = new Shader("TexturedVertex.glsl", "texturedfragment.glsl");

	if (!shader->LoadSuccess())
		return;

	filtering = true;
	repeating = false;
	init = true;
}

Renderer::~Renderer(void) {
	delete triangle;
	delete shader;
	glDeleteTextures(1, &texture);
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BindShader(shader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	triangle->Draw();
}