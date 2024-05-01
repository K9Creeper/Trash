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
	}

	void Start();

	bool running = true;
private:
	FloodColor groundColor;

public:
	Camera camera;
	World world;

	Render* render;

	void OnRender();
};
