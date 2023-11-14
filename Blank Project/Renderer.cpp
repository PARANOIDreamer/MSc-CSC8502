#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Heightmap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();
	heightMap = new HeightMap(TEXTUREDIR"noise1.png");
	planets = Mesh::LoadFromMeshFile("Sphere.msh");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds1.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	planetTexs[0] = SOIL_load_OGL_texture(TEXTUREDIR"planet.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	planetTexs[1] = SOIL_load_OGL_texture(TEXTUREDIR"sun.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	planetTexs[2] = SOIL_load_OGL_texture(TEXTUREDIR"sunbump.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	baseBump = SOIL_load_OGL_texture(TEXTUREDIR"Basebump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	robotMesh = Mesh::LoadFromMeshFile("Role_T.msh");
	anim = new MeshAnimation("Role_T.anm");
	robotMaterial = new MeshMaterial("Role_T.mat");
	LoadMesh(robotMesh, robotMaterial, 0);
	shipMesh = Mesh::LoadFromMeshFile("Spaceship_Base.msh");
	shipMaterial = new MeshMaterial("Spaceship_Base.mat");
	LoadMesh(shipMesh, shipMaterial, 1);

	if (!earthTex || !earthBump || !cubeMap || !waterTex || !planetTexs[0] || !planetTexs[1] || !planetTexs[2] || !baseBump) {
		return;
	}
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(planetTexs[0], true);
	SetTextureRepeating(planetTexs[1], true);
	SetTextureRepeating(planetTexs[2], true);
	SetTextureRepeating(baseBump, true);

	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("bumpVertex.glsl", "bumpFragment.glsl");
	modelShader = new Shader("meshVertex.glsl", "bumpFragment.glsl");
	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !modelShader->LoadSuccess()) {
		return;
	}

	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.55f, 2.0f, 0.7f));
	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 0.95, 1), heightmapSize.x);
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	waterRotate = 0.0f;
	waterCycle = 0.0f;
	currentFrame = 0;
	frameTime = 0.0f;
	robotPosition = Vector3(2050.0f, 50.0f, 2500.0f);
	direction = 0.0f;
	move = 0;
	camera_seat = 1;
	scene_no = 1;
	init = true;
}

Renderer ::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete modelShader;
	delete light;
	delete planets;
	delete robotMesh;
	delete anim;
	delete robotMaterial;
	delete shipMesh;
	delete shipMaterial;
}

void Renderer::UpdateScene(float dt) {
	waterRotate += dt * 2.0f;
	waterCycle += dt * 0.25f;
	float bit = -100.0f;
	if (scene_no == 1) {
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
	}
	else {

	}

	if (camera_seat == 2) {
		camera->UpdateCamera(0.1);
		viewMatrix = camera->BuildViewMatrix();
	}
	else {
		viewMatrix = Matrix4::Rotation(direction, Vector3(0, 1, 0)) * Matrix4::Rotation(90, Vector3(0, 1, 0)) * Matrix4::Translation(Vector3(-(robotPosition.x + bit), -100.0f, -2500.0f));
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_Q))
		camera_seat = 2;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_E))
		camera_seat = 1;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_1)) {
		if (scene_no != 1) {
			scene_no = 1;
			cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
		}
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_2)) {
		if (scene_no != 2) {
			scene_no = 2;
			cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"/Space/space_left.jpg", TEXTUREDIR"/Space/space_right.jpg", TEXTUREDIR"/Space/space_up.jpg", TEXTUREDIR"/Space/space_down.jpg", TEXTUREDIR"/Space/space_front.jpg", TEXTUREDIR"/Space/space_back.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
		}
	}
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	if (scene_no == 1) {
		DrawSkybox();
		DrawHeightmap();
		DrawWater();
		DrawMesh();
		DrawShip();
	}
	else {
		DrawSkybox();
		DrawShip();
		DrawPlanets();
	}
}

void Renderer::DrawMesh() {
	BindShader(modelShader);
	glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(modelShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);

	SetShaderLight(*light);
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
	BindShader(lightShader);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);

	SetShaderLight(*light);
	modelMatrix = Matrix4::Translation(Vector3(1800.0f, 205.0f, 2500.0f)) * Matrix4::Scale(Vector3(12.0f, 12.0f, 12.0f)) * Matrix4::Rotation(60, Vector3(0, 1, 0));
	UpdateShaderMatrices();

	for (int i = 0; i < shipMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[1][i]);
		shipMesh->DrawSubMesh(i);
	}
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();
	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

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
	modelMatrix = Matrix4::Translation(hSize * 0.48f) * Matrix4::Scale(hSize * 0.06f) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(3, 3, 3)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	quad->Draw();
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

void Renderer::DrawPlanets() {
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planetTexs[0]);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);

	modelMatrix = Matrix4::Translation(Vector3(500.0f, 200.0f, 1000.0f)) * Matrix4::Scale(Vector3(750.0f, 750.0f, 750.0f));// *Matrix4::Rotation(60, Vector3(0, 1, 0));
	UpdateShaderMatrices();

	planets->Draw();

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planetTexs[1]);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, planetTexs[2]);

	modelMatrix = Matrix4::Translation(Vector3(6500.0f, 180.0f, 3500.0f)) * Matrix4::Scale(Vector3(1000.0f, 1000.0f, 1000.0f));// *Matrix4::Rotation(60, Vector3(0, 1, 0));
	UpdateShaderMatrices();

	planets->Draw();
}