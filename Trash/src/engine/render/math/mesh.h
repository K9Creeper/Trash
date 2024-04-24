#pragma once
#include <vector>
#include "../floodgui/flood_gui_math.h"
using Vector3 = FloodVector3;

struct Triangle {
	Vector3 p[3];
};

struct Mesh {
	std::vector<Triangle>triangles;
};

