#pragma once
#include "math/mesh.h"
#include "math/matrix.h"

#include "world/world.h"
#include "camera/camera.h"

class Render;

class Engine {
private:
	Render* render;
public:
	Engine();
	~Engine() {
		delete render; delete pDepthBuffer;
	}

	void Start();

	bool running = true;
private:
	FloodColor groundColor;

	float* pDepthBuffer;
public:
	Camera camera;
	World world;

	void OnRender();
};