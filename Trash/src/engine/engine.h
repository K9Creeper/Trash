#pragma once
#include "math/mesh.h"
#include "math/matrix.h"

#include "camera/camera.h"

class Render;

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

	FloodColor groundColor;
public:
	Camera camera;
	void OnRender();
};