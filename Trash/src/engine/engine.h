#pragma once
#include "render/math/mesh.h"
#include "render/math/matrix.h"

class Render;
class Engine {
private:
	Render* render;
public:
	Engine();
	~Engine() { delete render; }

	void Start();
private:
	Mesh meshCube;
	Matrix4x4 matProj;

	Vector3 camera;
	Vector3 lookDir;
	float fYaw; // Cam rotate

	float fTheta;
public:
	void OnRender();
};