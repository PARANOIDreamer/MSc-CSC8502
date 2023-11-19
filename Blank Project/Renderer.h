#pragma once
#include "../nclgl/OGLRenderer.h"

class Camera;
class Shader;
class HeightMap;
class Mesh;
class MeshAnimation;
class MeshMaterial;

const int MODEL_NUM = 4;
const int Rain_NUM = 200;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawRobot();
	void DrawShip();
	void DrawPlanets();
	void DrawPlants();
	void DrawShadowScene();
	void DrawRain();
	void PresentScene();
	void DrawPostProcess();
	void UpdateLight();
	void DrawScene_n1();
	void DrawScene_n2();
	void GenerateScreenTexture(GLuint& into, bool depth = false);
	void LoadMesh(Mesh* mesh, MeshMaterial* material, int x);

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* modelShader;
	Shader* basicShader;
	Shader* groundShader;
	Shader* processShader;
	Shader* shadowShader;

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

	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint baseBump;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	vector <GLuint> matTextures[MODEL_NUM];
	GLuint sphereTexs[3];

	GLuint bufferFBO;
	GLuint processFBO;
	GLuint shadowFBO;
	GLuint bufferDepthTex;
	GLuint bufferColourTex[2];
	GLuint shadowTex;

	Vector3 robotPosition;
	Vector3 rainVert[Rain_NUM];
	Vector3 cameraPosition;
	Matrix4 cameraView;

	float waterRotate;
	float waterCycle;
	int currentFrame;
	float frameTime;
	float direction;
	float move;
	float sceneTime;
	int camera_seat;
	int scene_no;
	int proj_no;
	int screens;
	int rain_no;
	int timelight;
};