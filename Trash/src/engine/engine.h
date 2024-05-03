#pragma once
#include "math/mesh.h"
#include "math/matrix.h"

#include "world/world.h"
#include "camera/camera.h"

class Render;

class Engine {	
public:
	Engine();
	~Engine() {
		delete render;
		delete pDepthBuffer;
	}

	void Start();

	bool running = true;
private:
	FloodColor groundColor;

public:
	Camera camera;
	World world;

	float* pDepthBuffer = nullptr;

	Render* render;

	void OnRender();
};
