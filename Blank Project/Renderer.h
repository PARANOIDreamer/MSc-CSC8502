#pragma once
#include "../nclgl/OGLRenderer.h"

class Camera;
class Shader;
class HeightMap;
class Mesh;
class MeshAnimation;
class MeshMaterial;
const int MODEL_NUM = 32;

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

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* modelShader;

	HeightMap* heightMap;
	Mesh* quad;
	Mesh* robotMesh;
	MeshMaterial* robotMaterial;
	Mesh* shipMesh;
	MeshMaterial* shipMaterial;
	MeshAnimation* anim;

	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	vector <GLuint> matTextures[MODEL_NUM];

	float waterRotate;
	float waterCycle;
	int currentFrame;
	float frameTime;
	Vector3 robotPosition;
	float direction;
	float move;
	int camera_seat;
	int scene_no;
};