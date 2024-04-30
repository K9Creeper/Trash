#include "trace.h"

// Engine
#include "../engine.h"

bool intersectTriangle(Trace<Triangle>* tr, Triangle* triangle) {
    const float EPSILON = 0.0000001f;

    // Compute vectors for two edges of the triangle
    Vector3 edge1 = triangle->p[1] - triangle->p[0];
    Vector3 edge2 = triangle->p[2] - triangle->p[0];

    // Compute determinant to check if the ray and triangle are parallel
    Vector3 h = tr->direction.CrossProduct(edge2);
    float a = edge1.DotProduct(h);

    // Check if the ray is parallel to the triangle
    if (a > -EPSILON && a < EPSILON) {
        return false;
    }

    float f = 1.0f / a;
    Vector3 s = tr->origin - triangle->p[0];
    float u = f * s.DotProduct(h);

    // Check if the intersection point is outside the triangle
    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    Vector3 q = s.CrossProduct(edge1);
    float v = f * tr->direction.DotProduct(q);

    // Check if the intersection point is outside the triangle
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    float t = f * edge2.DotProduct(q);

    // Check if the intersection point is behind the ray origin
    if (t > EPSILON) {
        // Intersection point found
        tr->end = tr->origin + tr->direction * t;
        return true;
    }

    return false;
}

void Trace<Triangle>::TraceLine(Engine* engine,std::vector<Triangle*>& allTri) {
	collided = false;
	end = origin;

    float flClose = 99999.99f;

	for (Triangle* tri : allTri) {
        if (intersectTriangle(this, tri))
        {
            float dst = ((Vector3)(origin - tri->findTriangleCenter())).Length();
            if (dst < flClose) {
                collided = true;
                hit = *tri;
                flClose = dst;
            }
        }
	}
}