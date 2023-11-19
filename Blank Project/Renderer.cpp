#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Heightmap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"

#define SHADOWSIZE 2048
int POST_PASSES = 10;

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();
	heightMap = new HeightMap(TEXTUREDIR"noise1.png");
	spheres = Mesh::LoadFromMeshFile("Sphere.msh");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds1.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sphereTexs[0] = SOIL_load_OGL_texture(TEXTUREDIR"Shield.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sphereTexs[1] = SOIL_load_OGL_texture(TEXTUREDIR"sun.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sphereTexs[2] = SOIL_load_OGL_texture(TEXTUREDIR"planet.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	baseBump = SOIL_load_OGL_texture(TEXTUREDIR"Basebump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	robotMesh = Mesh::LoadFromMeshFile("Role_T.msh");
	anim = new MeshAnimation("Role_T.anm");
	robotMaterial = new MeshMaterial("Role_T.mat");
	LoadMesh(robotMesh, robotMaterial, 0);
	shipMesh = Mesh::LoadFromMeshFile("Spaceship_Base.msh");
	shipMaterial = new MeshMaterial("Spaceship_Base.mat");
	LoadMesh(shipMesh, shipMaterial, 1);
	treeMesh = Mesh::LoadFromMeshFile("Tree1.msh");
	treeMaterial = new MeshMaterial("Tree1.mat");
	LoadMesh(treeMesh, treeMaterial, 2);
	grassMesh = Mesh::LoadFromMeshFile("GrassE.msh");
	grassMaterial = new MeshMaterial("GrassE.mat");
	LoadMesh(grassMesh, grassMaterial, 3);

	if (!earthTex || !earthBump || !cubeMap || !waterTex || !sphereTexs[0] || !sphereTexs[1] || !sphereTexs[2] || !baseBump) {
		return;
	}
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(sphereTexs[0], true);
	SetTextureRepeating(sphereTexs[1], true);
	SetTextureRepeating(sphereTexs[2], true);
	SetTextureRepeating(baseBump, true);

	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("bumpVertex.glsl", "bumpFragment.glsl");
	modelShader = new Shader("meshVertex.glsl", "bumpFragment.glsl");
	basicShader = new Shader("modelVertex.glsl", "TexturedFragment.glsl");
	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !modelShader->LoadSuccess() || !basicShader->LoadSuccess()) {
		return;
	}
	groundShader = new Shader("shadowscenevert.glsl", "spectFragment.glsl");
	processShader = new Shader("modelVertex.glsl", "processfrag.glsl");
	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");
	if (!groundShader->LoadSuccess() || !processShader->LoadSuccess() || !shadowShader->LoadSuccess()) {
		return;
	}

	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.55f, 2.0f, 0.7f));
	light = new Light(Vector3(2000.0f, 500.0f, 2000.0f), Vector4(1, 1, 0.95, 1), 3000);
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	waterRotate = 0.0f;
	waterCycle = 0.0f;
	currentFrame = 0;
	frameTime = 0.0f;
	robotPosition = Vector3(2050.0f, 50.0f, 2500.0f);

	cameraPosition = camera->GetPosition();
	for (int i = 0; i < Rain_NUM; ++i) {
		rainVert[i] = Vector3(cameraPosition.x - 300.0f, 300.0f, cameraPosition.z);
		rainVert[i].x += (float)(rand() % 1000);
		rainVert[i].y += (float)(rand() % 400);
		rainVert[i].z -= (float)(rand() % 1000);
	}

	direction = 0.0f;
	move = 0;
	sceneTime = 0.0f;
	camera_seat = 1;
	scene_no = 1;
	rain_no = 0;
	screens = 1;
	timelight = 0;
	proj_no = 0;
	init = true;

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);
	glGenFramebuffers(1, &shadowFBO);

	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex[0]);
	GenerateScreenTexture(bufferColourTex[1]);
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0]) {
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

Renderer ::~Renderer(void) {
	delete camera;
	delete light;
	delete heightMap;
	delete quad;
	delete spheres;

	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete modelShader;
	delete basicShader;
	delete groundShader;
	delete processShader;
	delete shadowShader;

	delete robotMesh;
	delete anim;
	delete robotMaterial;
	delete shipMesh;
	delete shipMaterial;
	delete treeMesh;
	delete treeMaterial;
	delete grassMesh;
	delete grassMaterial;

	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);
	glDeleteFramebuffers(1, &shadowFBO);
}

void Renderer::UpdateScene(float dt) {
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R)) {
		if (scene_no == 1) {
			if (screens == 1)
				screens = 2;
			else
				screens = 1;
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Q))
		camera_seat = 2;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_E))
		camera_seat = 1;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
		if (scene_no != 1) {
			scene_no = 1;
			rain_no = 0;
			cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
			projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
		}
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_2)) {
		cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"/Space/space_left.jpg", TEXTUREDIR"/Space/space_right.jpg", TEXTUREDIR"/Space/space_up.jpg", TEXTUREDIR"/Space/space_down.jpg", TEXTUREDIR"/Space/space_front.jpg", TEXTUREDIR"/Space/space_back.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
		if (scene_no != 2) {
			projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
			scene_no = 2;
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3))
		rain_no = (rain_no == 0) ? 1 : 0;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4)) {
		if (scene_no == 1)
			timelight = (timelight == 0) ? 1 : 0;
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_5))
		POST_PASSES = (POST_PASSES == 10) ? 0 : 10;
	float bit = -100.0f;
	if (scene_no == 1) {
		waterRotate += dt * 2.0f;
		waterCycle += dt * 0.25f;
		UpdateLight();
		frameTime -= dt;
		while (frameTime < 0.0f) {
			currentFrame = (currentFrame + 1) % anim->GetFrameCount();
			frameTime += 1.0f / anim->GetFrameRate();
		}
		float robotSpeed = 0.2f;
		if (move > 100) {
			direction = 180.0f;
			robotPosition.x -= robotSpeed;
			bit = 100.0f;
			if (robotPosition.x <= 2050.0f)move = 0;
		}
		else {
			move += robotSpeed;
			direction = 0.0f;
			bit = -100.0f;
			robotPosition.x += robotSpeed;
		}
		if (rain_no == 1) {
			Vector3 cameraPos = camera->GetPosition();
			float m_x = cameraPosition.x - cameraPos.x;
			cameraPosition.x = cameraPos.x;
			float m_z = cameraPosition.z - cameraPos.z;
			cameraPosition.z = cameraPos.z;
			for (int i = 0; i < Rain_NUM; i++) {
				if (rainVert[i].y > 50.0f)
					rainVert[i].y -= robotSpeed * 10;
				else
					rainVert[i].y = 300.0f + (float)(rand() % 400);
				rainVert[i].x -= m_x;
				rainVert[i].z -= m_z;
			}
		}
		cameraView = Matrix4::Rotation(direction, Vector3(0, 1, 0)) * Matrix4::Rotation(90, Vector3(0, 1, 0)) * Matrix4::Translation(Vector3(-(robotPosition.x + bit), -100.0f, -2500.0f));
	}
	else {
		timelight = 0;
		sceneTime += dt;
		cameraView = Matrix4::Rotation(90, Vector3(0, 1, 0)) * Matrix4::Translation(Vector3(-(robotPosition.x + bit), -100.0f, -2500.0f));
	}
	if (screens == 1) {
		if (camera_seat == 2) {
			camera->UpdateCamera(0.1);
			viewMatrix = camera->BuildViewMatrix();
		}
		else
			viewMatrix = cameraView;
	}
	else
		camera->UpdateCamera(0.1);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	if (scene_no == 1) {
		if (screens == 1) {
			glViewport(0, 0, (float)width, (float)height);
			proj_no = 0;
			DrawScene_n1();
		}
		else {
			glViewport(0, 0, (float)width / 2, (float)height);
			proj_no = 1;
			camera_seat = 1;
			DrawScene_n1();

			glViewport((float)width / 2, 0, (float)width, (float)height);
			proj_no = 2;
			camera_seat = 2;
			DrawScene_n1();
		}
	}
	else {
		glViewport(0, 0, (float)width, (float)height);
		proj_no = 0;
		DrawScene_n2();
	}

}

void Renderer::UpdateLight() {
	Vector3 lightpos = light->GetPosition();
	if (timelight == 0)
		light->SetPosition(Vector3(2000.0f, 500.0f, 2000.0f));
	else if (timelight == 1) {
		light->SetPosition(Vector3(lightpos.x + 2.0f, 500.0f, lightpos.z + 2.0f));
		if (light->GetPosition().x > 4000)
			timelight = 2;
	}
	else {
		light->SetPosition(Vector3(lightpos.x - 2.0f, 500.0f, lightpos.z - 2.0f));
		if (light->GetPosition().x < 0)
			timelight = 1;
	}
}

void Renderer::DrawScene_n1() {
	DrawSkybox();
	DrawShadowScene();
	DrawHeightmap();
	DrawWater();
	DrawRobot();
	DrawShip();
	DrawPlants();
	if (rain_no == 1)
		DrawRain();
}

void Renderer::DrawScene_n2() {
	DrawSkybox();
	DrawShip();
	DrawPlanets();
	DrawPostProcess();
	PresentScene();
}

void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(1950.0f, 0.0f, 2500.0f));
	projMatrix = Matrix4::Perspective(1.0f, 1500.0f, 1, 45.0f);
	shadowMatrix = projMatrix * viewMatrix;

	modelMatrix = Matrix4::Translation(Vector3(1800.0f, 205.0f, 2500.0f)) * Matrix4::Scale(Vector3(12.0f, 12.0f, 12.0f)) * Matrix4::Rotation(60, Vector3(0, 1, 0));
	UpdateShaderMatrices();
	for (int i = 0; i < shipMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[1][i]);
		shipMesh->DrawSubMesh(i);
	}

	modelMatrix = Matrix4::Translation(robotPosition) * Matrix4::Scale(Vector3(20.0f, 20.0f, 20.0f)) * Matrix4::Rotation(90, Vector3(0, 1, 0)) * Matrix4::Rotation(direction, Vector3(0, 1, 0));
	UpdateShaderMatrices();
	for (int i = 0; i < robotMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[0][i]);
		robotMesh->DrawSubMesh(i);
	}

	modelMatrix = Matrix4::Translation(Vector3(2140.0f, 50.0f, 2600.0f)) * Matrix4::Scale(Vector3(12.0f, 12.0f, 12.0f));
	UpdateShaderMatrices();
	for (int i = 0; i < treeMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[2][i]);
		treeMesh->DrawSubMesh(i);
	}
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 5; j++) {
			modelMatrix = Matrix4::Translation(Vector3(2100.0f - i * 20.0f, 50.0f, 2600.0f + j * 20.0f)) * Matrix4::Scale(Vector3(12.0f, 12.0f, 12.0f));
			UpdateShaderMatrices();
			for (int i = 0; i < grassMesh->GetSubMeshCount(); ++i) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, matTextures[3][i]);
				grassMesh->DrawSubMesh(i);
			}
		}
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	if (proj_no == 0)
		glViewport(0, 0, width, height);
	else if (proj_no == 1)
		glViewport(0, 0, (float)width / 2, (float)height);
	else
		glViewport((float)width / 2, 0, (float)width, (float)height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawRobot() {
	BindShader(modelShader);
	glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(modelShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);

	SetShaderLight(*light);
	if (camera_seat == 2) {
		viewMatrix = camera->BuildViewMatrix();
	}
	else
		viewMatrix = cameraView;
	if (proj_no == 0 || proj_no == 2)
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	else
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height / 2, 45.0f);
	modelMatrix = Matrix4::Translation(robotPosition) * Matrix4::Scale(Vector3(20.0f, 20.0f, 20.0f)) * Matrix4::Rotation(90, Vector3(0, 1, 0)) * Matrix4::Rotation(direction, Vector3(0, 1, 0));
	UpdateShaderMatrices();

	vector <Matrix4> frameMatrices;
	const Matrix4* invBindPose = robotMesh->GetInverseBindPose();
	const Matrix4* frameData = anim->GetJointData(currentFrame);
	for (unsigned int i = 0; i < robotMesh->GetJointCount(); ++i) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}
	int j = glGetUniformLocation(modelShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());
	for (int i = 0; i < robotMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[0][i]);
		robotMesh->DrawSubMesh(i);
	}
}

void Renderer::DrawShip() {
	if (scene_no == 1) {
		BindShader(lightShader);
		glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
		glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
		glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, baseBump);

		if (camera_seat == 2) {
			viewMatrix = camera->BuildViewMatrix();
		}
		else
			viewMatrix = cameraView;
		if (proj_no == 0 || proj_no == 2)
			projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
		else
			projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height / 2, 45.0f);

		modelMatrix = Matrix4::Translation(Vector3(1800.0f, 200.0f, 2500.0f)) * Matrix4::Scale(Vector3(12.0f, 12.0f, 12.0f)) * Matrix4::Rotation(60, Vector3(0, 1, 0));
		UpdateShaderMatrices();

		for (int i = 0; i < shipMesh->GetSubMeshCount(); ++i) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, matTextures[1][i]);
			shipMesh->DrawSubMesh(i);
		}
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
		BindShader(lightShader);
		glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
		glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
		glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, baseBump);

		SetShaderLight(*light);
		if (camera_seat == 2) {
			viewMatrix = camera->BuildViewMatrix();
		}
		else
			viewMatrix = cameraView;
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
		modelMatrix = Matrix4::Translation(Vector3(1800.0f, 205.0f, 2500.0f)) * Matrix4::Scale(Vector3(12.0f, 12.0f, 12.0f)) * Matrix4::Rotation(60, Vector3(0, 1, 0));
		UpdateShaderMatrices();

		for (int i = 0; i < shipMesh->GetSubMeshCount(); ++i) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, matTextures[1][i]);
			shipMesh->DrawSubMesh(i);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Renderer::DrawPlants() {
	BindShader(lightShader);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);

	SetShaderLight(*light);
	if (camera_seat == 2) {
		viewMatrix = camera->BuildViewMatrix();
	}
	else
		viewMatrix = cameraView;
	if (proj_no == 0 || proj_no == 2)
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	else
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height / 2, 45.0f);
	modelMatrix = Matrix4::Translation(Vector3(2140.0f, 50.0f, 2600.0f)) * Matrix4::Scale(Vector3(12.0f, 12.0f, 12.0f));
	UpdateShaderMatrices();
	for (int i = 0; i < treeMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[2][i]);
		treeMesh->DrawSubMesh(i);
	}

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 5; j++) {
			modelMatrix = Matrix4::Translation(Vector3(2100.0f - i * 20.0f, 50.0f, 2600.0f + j * 20.0f)) * Matrix4::Scale(Vector3(12.0f, 12.0f, 12.0f));
			UpdateShaderMatrices();
			for (int i = 0; i < grassMesh->GetSubMeshCount(); ++i) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, matTextures[3][i]);
				grassMesh->DrawSubMesh(i);
			}
		}
	}
	BindShader(groundShader);
	glUniform1i(glGetUniformLocation(groundShader->GetProgram(), "diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(groundShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphereTexs[0]);
	glUniform1i(glGetUniformLocation(groundShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);
	glUniform1i(glGetUniformLocation(groundShader->GetProgram(), "shadowTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	modelMatrix = Matrix4::Translation(Vector3(2155.0f, 50.0f, 2625.0f)) * Matrix4::Scale(Vector3(105.0f, 105.0f, 105.0f));
	UpdateShaderMatrices();
	spheres->Draw();
}

void Renderer::DrawSkybox() {
	if (scene_no == 1) {
		glDepthMask(GL_FALSE);
		BindShader(skyboxShader);
		if (camera_seat == 2) {
			viewMatrix = camera->BuildViewMatrix();
		}
		else
			viewMatrix = cameraView;
		if (proj_no == 0 || proj_no == 2)
			projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
		UpdateShaderMatrices();

		quad->Draw();
		glDepthMask(GL_TRUE);
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDepthMask(GL_FALSE);
		BindShader(skyboxShader);
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
		UpdateShaderMatrices();

		quad->Draw();
		glDepthMask(GL_TRUE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Renderer::DrawHeightmap() {
	BindShader(groundShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(groundShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(groundShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);
	glUniform1i(glGetUniformLocation(groundShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);
	glUniform1i(glGetUniformLocation(groundShader->GetProgram(), "shadowTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	if (camera_seat == 2) {
		viewMatrix = camera->BuildViewMatrix();
	}
	else
		viewMatrix = cameraView;
	if (proj_no == 0 || proj_no == 2)
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();
	heightMap->Draw();
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

	Vector3 hSize = heightMap->GetHeightmapSize();
	if (camera_seat == 2) {
		viewMatrix = camera->BuildViewMatrix();
	}
	else
		viewMatrix = cameraView;
	if (proj_no == 0 || proj_no == 2)
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	modelMatrix = Matrix4::Translation(hSize * 0.48f) * Matrix4::Scale(hSize * 0.06f) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(3, 3, 3)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	quad->Draw();
}

void Renderer::DrawRain() {
	BindShader(lightShader);
	Light& l = light[0];
	SetShaderLight(l);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphereTexs[0]);

	if (camera_seat == 2) {
		viewMatrix = camera->BuildViewMatrix();
	}
	else
		viewMatrix = cameraView;
	if (proj_no == 0 || proj_no == 2)
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	for (int i = 0; i < Rain_NUM; ++i) {
		modelMatrix = Matrix4::Translation(rainVert[i]);
		UpdateShaderMatrices();
		spheres->Draw();
	}
}

void Renderer::DrawPlanets() {
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	BindShader(lightShader);
	Light& l = light[0];
	SetShaderLight(l);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphereTexs[2]);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);

	if (camera_seat == 2) {
		viewMatrix = camera->BuildViewMatrix();
	}
	else
		viewMatrix = cameraView;
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	modelMatrix = Matrix4::Translation(Vector3(500.0f, 200.0f, 1000.0f)) * Matrix4::Scale(Vector3(750.0f, 750.0f, 750.0f)) * Matrix4::Rotation(sceneTime * 10, Vector3(0, 1, 0));
	UpdateShaderMatrices();
	spheres->Draw();

	BindShader(basicShader);
	glUniform1i(glGetUniformLocation(basicShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphereTexs[1]);
	modelMatrix = Matrix4::Translation(Vector3(6500.0f, 180.0f, 3500.0f)) * Matrix4::Scale(Vector3(1000.0f, 1000.0f, 1000.0f)) * Matrix4::Rotation(sceneTime * 10, Vector3(0, 1, 0));
	UpdateShaderMatrices();
	spheres->Draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawPostProcess() {
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(processShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);
	for (int i = 0; i < POST_PASSES; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
		quad->Draw();
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
		quad->Draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::PresentScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(basicShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(basicShader->GetProgram(), "diffuseTex"), 0);
	quad->Draw();
}

void Renderer::GenerateScreenTexture(GLuint& into, bool depth) {
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH24_STENCIL8 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_STENCIL : GL_RGBA;
	GLuint bite = depth ? GL_UNSIGNED_INT_24_8 : GL_UNSIGNED_BYTE;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type, bite, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::LoadMesh(Mesh* mesh, MeshMaterial* material, int x) {
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures[x].emplace_back(texID);
	}
}