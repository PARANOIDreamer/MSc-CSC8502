#pragma once
#include "../nclgl/OGLRenderer.h"

class Camera;
class Shader;
class HeightMap;
class Mesh;
class MeshAnimation;
class MeshMaterial;

const int MODEL_NUM = 5;
const int Rain_NUM = 200;

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
	void DrawRain();

	void UpdateLight();
	void DrawScene_n1();
	void DrawScene_n2();

	void PresentScene();
	void DrawPostProcess();
	//void DrawSpotlights();
	void GenerateScreenTexture(GLuint& into, bool depth = false);

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* modelShader;
	Shader* basicShader;
	Shader* groundShader;
	Shader* processShader;
	//Shader* rainShader;

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
	//Mesh* spotLight;
	//Mesh* rain;

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
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;
	//GLuint rainTex;

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
	int screens;
	int rain_no;
	int timelight;

	/*GLuint shadowTex;
	GLuint shadowFBO;
	Shader* shadowShader;
	Shader* sceneShader;*/
};