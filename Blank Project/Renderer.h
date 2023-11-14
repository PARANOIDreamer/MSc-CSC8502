#pragma once
#include "../nclgl/OGLRenderer.h"

class Camera;
class Shader;
class HeightMap;
class Mesh;
class MeshAnimation;
class MeshMaterial;
const int MODEL_NUM = 5;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;
	void LoadMesh(Mesh* mesh, MeshMaterial* material, int x);

protected:
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawMesh();
	void DrawShip();
	void DrawPlanets();
	void DrawPlants();
	void DrawShadowScene();
	//void DrawSpotlights();

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* modelShader;
	Shader* basicShader;
	Shader* spotlightShader;

	HeightMap* heightMap;
	Mesh* quad;
	Mesh* robotMesh;
	MeshMaterial* robotMaterial;
	Mesh* shipMesh;
	MeshMaterial* shipMaterial;
	Mesh* treeMesh;
	MeshMaterial* treeMaterial;
	Mesh* grassMesh;
	MeshMaterial* grassMaterial;
	MeshAnimation* anim;
	Mesh* spheres;
	Mesh* spotLight;

	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint baseBump;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	vector <GLuint> matTextures[MODEL_NUM];
	GLuint sphereTexs[2];

	float waterRotate;
	float waterCycle;
	int currentFrame;
	float frameTime;
	Vector3 robotPosition;
	float direction;
	float move;
	float sceneTime;
	int camera_seat;
	int scene_no;

	GLuint shadowTex;
	GLuint shadowFBO;
	Shader* shadowShader;
	Shader* sceneShader;
};