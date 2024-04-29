#include "trace.h"

// For struct Triangle
#include "../math/mesh.h"

float GetDistance(vec v1, vec v2)
{
	if ((v1.x == 0) && (v2.x == 0) && (v1.y == 0) && (v2.y == 0) && (v1.z == 0) &&
		(v2.z == 0))
		return 0.0f;

	return v1.dist(v2);
}

void TraceLine(Trace<Triangle>* tr, std::vector<Triangle>& allTri) {
	tr->collided = false;
	tr->end = tr->origin;
	static float flNearestDist, flDist;

	flNearestDist = 9999.99f;
	for (const Triangle& tri : allTri) {
		for (int i = 0; i < 3; i++) {
			flDist = (tr->origin - tri.p[i]);
		}
	}
}