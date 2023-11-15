#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Heightmap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"

//#define SHADOWSIZE 2048
Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();
	heightMap = new HeightMap(TEXTUREDIR"noise1.png");
	spheres = Mesh::LoadFromMeshFile("Sphere.msh");
	//spotLight= Mesh::LoadFromMeshFile("Cone.msh");

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
	groudShader = new Shader("bumpVertex.glsl", "spectFragment.glsl");
	//spotlightShader = new Shader("pointlightvert.glsl", "pointlightfrag.glsl");

	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !modelShader->LoadSuccess() || !basicShader->LoadSuccess()) {
		return;
	}

	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.55f, 2.0f, 0.7f));
	light = new Light[3];
	light[0] = Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 0.95, 1), 2500);
	//light[1] = Light(Vector3(2000.0f, 180.0f, 3500.0f), Vector4(1, 0.8, 0, 1), heightmapSize.x);
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	/*sceneShader = new Shader("shadowscenevert.glsl", "shadowscenefrag.glsl");
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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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

	delete[] light;
	delete spheres;

	delete robotMesh;
	delete anim;
	delete robotMaterial;
	delete shipMesh;
	delete shipMaterial;
	delete treeMesh;
	delete treeMaterial;
	delete grassMesh;
	delete grassMaterial;
}

void Renderer::UpdateScene(float dt) {
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R)) {
		if (screens == 1) {
			screens = 2;
		}
		else {
			screens = 1;
			projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Q))
		camera_seat = 2;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_E))
		camera_seat = 1;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
		if (scene_no != 1) {
			scene_no = 1;
			cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2)) {
		if (scene_no != 2) {
			scene_no = 2;
			cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"/Space/space_left.jpg", TEXTUREDIR"/Space/space_right.jpg", TEXTUREDIR"/Space/space_up.jpg", TEXTUREDIR"/Space/space_down.jpg", TEXTUREDIR"/Space/space_front.jpg", TEXTUREDIR"/Space/space_back.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
		}
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3)) {
		rain_no = (rain_no == 0) ? 1 : 0;
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4)) {
		if(scene_no==1)
		timelight = (timelight == 0) ? 1 : 0;
	}

	waterRotate += dt * 2.0f;
	waterCycle += dt * 0.25f;
	float bit = -100.0f;
	if (scene_no == 1) {
		if (timelight == 0) {
			light[0].SetRadius(2500);
		}
		else if (timelight == 1) {
			light[0].SetRadius(light[0].GetRadius() - 10);
			if (light[0].GetRadius() < 800)
				timelight = 2;
		}
		else {
			light[0].SetRadius(light[0].GetRadius() + 5);
			if (light[0].GetRadius() > 2500)
				timelight = 1;
		}
		
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
		Vector3 cameraPos = camera->GetPosition();
		float m_x = cameraPosition.x - cameraPos.x;
		cameraPosition.x = cameraPos.x;
		float m_z = cameraPosition.z - cameraPos.z;
		cameraPosition.z = cameraPos.z;
		for (int i = 0; i < Rain_NUM; i++) {
			if (rainVert[i].y > 50.0f) {
				rainVert[i].y -= robotSpeed * 10;
			}
			else
			{
				rainVert[i].y = 300.0f + (float)(rand() % 400);
			}
			rainVert[i].x -= m_x;
			rainVert[i].z -= m_z;
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
		else {
			viewMatrix = cameraView;
		}
	}
	else
	{
		camera->UpdateCamera(0.1);
	}
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	if (screens == 1) {
		glViewport(0, 0, (float)width, (float)height);
		if (scene_no == 1) {
			DrawSkybox();
			DrawHeightmap();
			DrawWater();
			DrawMesh();

			//DrawShadowScene();
			DrawShip();
			DrawPlants();
			if (rain_no == 1) {
				DrawRain();
			}

		}
		else {
			DrawSkybox();
			DrawShip();
			DrawPlanets();
		}
	}
	else {
		glViewport(0, 0, (float)width / 2, (float)height);
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height / 2, 45.0f);
		viewMatrix = cameraView;
		if (scene_no == 1) {
			DrawSkybox();
			DrawHeightmap();
			DrawWater();
			DrawMesh();

			//DrawShadowScene();
			DrawShip();
			DrawPlants();
			if (rain_no == 1) {
				DrawRain();
			}

		}
		else {
			DrawSkybox();
			DrawShip();
			DrawPlanets();
		}

		glViewport((float)width / 2, 0, (float)width, (float)height);
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
		viewMatrix = camera->BuildViewMatrix();
		if (scene_no == 1) {
			DrawSkybox();
			DrawHeightmap();
			DrawWater();
			DrawMesh();

			//DrawShadowScene();
			DrawShip();
			DrawPlants();
			if (rain_no == 1) {
				DrawRain();
			}

		}
		else {
			DrawSkybox();
			DrawShip();
			DrawPlanets();
		}
	}

}

//void Renderer::DrawShadowScene() {
//	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
//
//	glClear(GL_DEPTH_BUFFER_BIT);
//	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
//	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
//
//	BindShader(shadowShader);
//	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
//	projMatrix = Matrix4::Perspective(1, 100, 1, 45);
//	shadowMatrix = projMatrix * viewMatrix;
//	
//	
//	modelMatrix = Matrix4::Translation(Vector3(1800.0f, 205.0f, 2500.0f)) * Matrix4::Scale(Vector3(12.0f, 12.0f, 12.0f)) * Matrix4::Rotation(60, Vector3(0, 1, 0));
//	UpdateShaderMatrices();
//	for (int i = 0; i < shipMesh->GetSubMeshCount(); ++i) {
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, matTextures[1][i]);
//		shipMesh->DrawSubMesh(i);
//	}
//
//	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//	glViewport(0, 0, width, height);
//
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}

void Renderer::DrawMesh() {
	BindShader(modelShader);
	glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(modelShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);

	Light& l = light[0];
	SetShaderLight(l);
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
	//glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex"), 2);
	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, shadowTex);

	Light& l = light[0];
	SetShaderLight(l);
	modelMatrix = Matrix4::Translation(Vector3(1800.0f, 205.0f, 2500.0f)) * Matrix4::Scale(Vector3(12.0f, 12.0f, 12.0f)) * Matrix4::Rotation(60, Vector3(0, 1, 0));
	UpdateShaderMatrices();

	for (int i = 0; i < shipMesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[1][i]);
		shipMesh->DrawSubMesh(i);
	}
}

void Renderer::DrawPlants() {
	BindShader(lightShader);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);

	Light& l = light[0];
	SetShaderLight(l);
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

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphereTexs[0]);
	modelMatrix = Matrix4::Translation(Vector3(2155.0f, 50.0f, 2625.0f)) * Matrix4::Scale(Vector3(105.0f, 105.0f, 105.0f));
	UpdateShaderMatrices();
	spheres->Draw();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();
	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	BindShader(groudShader);
	Light& l = light[0];
	SetShaderLight(l);
	glUniform3fv(glGetUniformLocation(groudShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(groudShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);
	glUniform1i(glGetUniformLocation(groudShader->GetProgram(), "bumpTex"), 1);
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
	Light& l = light[0];
	SetShaderLight(l);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphereTexs[2]);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, baseBump);

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
}

//void Renderer::DrawSpotlights() {
//	BindShader(spotlightShader);
//
//	glUniform1i(glGetUniformLocation(spotlightShader->GetProgram(), "depthTex"), 0);
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
//
//	glUniform1i(glGetUniformLocation(spotlightShader->GetProgram(), "normTex"), 1);
//	glActiveTexture(GL_TEXTURE1);
//	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);
//
//	glUniform3fv(glGetUniformLocation(spotlightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
//
//	glUniform2f(glGetUniformLocation(spotlightShader->GetProgram(), "pixelSize"), 1.0f / width, 1.0f / height);
//
//	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
//	glUniformMatrix4fv(glGetUniformLocation(spotlightShader->GetProgram(), "inverseProjView"), 1, false, invViewProj.values);
//	UpdateShaderMatrices();
//		Light& l = light[1];
//		SetShaderLight(l);
//		spotLight->Draw();
//}

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

	for (int i = 0; i < Rain_NUM; ++i) {
		modelMatrix = Matrix4::Translation(rainVert[i]);
		UpdateShaderMatrices();
		spheres->Draw();
	}

}