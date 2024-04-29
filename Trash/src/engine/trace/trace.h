#pragma once
#include "../math/vector.h"

template <typename T>
struct Trace {
	Vector3 origin;
	Vector3 end;

	bool collided = false;
	T* hit = nullptr;
};

struct Triangle;
void TraceLine(Trace<Triangle>& tr, std::vector<Triangle>& allTri);