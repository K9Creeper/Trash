#pragma once
#include <vector>
#include "../math/vector.h"
#include "../math/mesh.h"

class Engine;

template <typename T>
struct Trace {
	Vector3 origin;
	Vector3 direction;
	Vector3 end;

	bool collided = false;
	T hit;

	void TraceLine(Engine* engine, std::vector<T>& allTri);
};