#pragma once
#include "render/math/mesh.h"
#include "render/math/matrix.h"

class Render;

struct ProjectionMatrix {
	int fFovDegrees;
	float fAspectRatio, 
		fNear, 
		fFar;

	Matrix4x4 matrix;

	Matrix4x4& Create();
};

class Camera {
public:
	void Create(const float& fov);
	void CreateCameraMatrix();


	ProjectionMatrix matProj;
	Matrix4x4 matView;

	FloodVector3 origin;	// Location of camera in world space
	FloodVector3 lookDir;	// Direction vector along the direction camera points
	float fYaw;		// FPS Camera rotation in XZ plane
	float fPitch;	// FPS Camera rotation in XZ plane
};

class Engine {
private:
	Render* render;
public:
	Engine();
	~Engine() { delete render; }

	void Start();

	bool running = true;
private:
	Mesh meshCube;

	Matrix4x4 matWorld;

public:
	Camera camera;
	void OnRender();
};