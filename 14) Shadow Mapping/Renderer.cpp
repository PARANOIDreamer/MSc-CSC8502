#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"

#define SHADOWSIZE 4096//2048

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	camera = new Camera(-90.0f, 315.0f, Vector3(-8.0f, 1500.0f, 8.0f));
	//light = new Light(Vector3(-20.0f, 10.0f, -20.0f), Vector4(1, 1, 1, 1), 250.0f);
	light = new Light(Vector3(-200.0f, 100.0f, -200.0f), Vector4(1, 1, 1, 1), 2500.0f);
	//light = new Light(Vector3(-20.0f, 100.0f, -20.0f), Vector4(1, 1, 1, 1), 2500.0f);
	//camera = new Camera(-45.0f, 0.0f, Vector3(1800.0f, 1500.0f, 2600.0f));
	//light = new Light(Vector3(-500.0f, 500.0f, -500.0f), Vector4(1, 1, 1, 1), 6000);
	sceneShader = new Shader("shadowscenevert.glsl", "shadowscenefrag.glsl");
	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");

	if (!sceneShader->LoadSuccess() || !shadowShader->LoadSuccess()) {
		return;
	}
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	sceneMeshes.emplace_back(Mesh::GenerateQuad());
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Sphere.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cylinder.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cone.msh"));

	sceneDiffuse = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sceneBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(sceneDiffuse, true);
	SetTextureRepeating(sceneBump, true);
	glEnable(GL_DEPTH_TEST);

	sceneTransforms.resize(4);
	//sceneTransforms[0] = Matrix4::Rotation(90, Vector3(1, 0, 0)) * Matrix4::Scale(Vector3(1000, 1000, 1));
	sceneTransforms[0] = Matrix4::Translation(Vector3(500.0f, 0.0f, 500.0f)) * Matrix4::Scale(Vector3(1000.0f, 1000.0f, 1000.0f)) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	sceneTime = 0.0f;
	init = true;
}

Renderer ::~Renderer(void) {
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);

	for (auto& i : sceneMeshes) {
		delete i;
	}
	delete camera;
	delete sceneShader;
	delete shadowShader;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt*10);
	sceneTime += dt;

	for (int i = 1; i < 4; ++i) {
		Vector3 t = Vector3(-100 + (50 * i), 10.0f + sin(sceneTime * i), 0) +Vector3(500.0f, 5.0f, 500.0f); //(1850.0f, 0.0f, 2500.0f);
		//Vector3 t = Vector3(-10 + (5 * i), 2.0f + sin(sceneTime * i), 0);
		//sceneTransforms[i] = Matrix4::Translation(t) * Matrix4::Rotation(sceneTime * 10 * i, Vector3(1, 0, 0));
		sceneTransforms[i] = Matrix4::Translation(t) * Matrix4::Rotation(sceneTime * 10 * i, Vector3(1, 0, 0)) * Matrix4::Scale(Vector3(10.0f, 10.0f, 10.0f));
	}
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawShadowScene();
	DrawMainScene();
}

void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(500, 5.0, 500));// *Matrix4::Rotation(-30, Vector3(1, 0, 0));
	projMatrix = Matrix4::Perspective(1, 10000.0f, 1, 45);
	shadowMatrix = projMatrix * viewMatrix;

	for (int i = 0; i < 4; ++i) {
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
		sceneMeshes[i]->Draw();
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawMainScene() {
	BindShader(sceneShader);
	SetShaderLight(*light);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex"), 2);
	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneDiffuse);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sceneBump);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	for (int i = 0; i < 4; ++i) {
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
		sceneMeshes[i]->Draw();
	}
}