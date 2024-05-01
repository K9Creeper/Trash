#include "trace.h"

// Engine
#include "../engine.h"

bool intersectTriangle(Trace<Triangle>* tr, const Triangle& triangle) {
    const float EPSILON = 0.0000001f;

    //  two edges of the triangle
    Vector3 edge1 = Vector3(triangle.p[1]) - triangle.p[0];
    Vector3 edge2 = Vector3(triangle.p[2]) - triangle.p[0];

    Vector3 h = tr->direction.CrossProduct(edge2);
    float a = edge1.DotProduct(h);

    // parallel to the triangle
    if (a > -EPSILON && a < EPSILON) {
        return false;
    }

    float f = 1.0f / a;
    Vector3 s = tr->origin - triangle.p[0];
    float u = f * s.DotProduct(h);

    //  intersection point is outside the triangle
    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    Vector3 q = s.CrossProduct(edge1);
    float v = f * tr->direction.DotProduct(q);

    //  intersection point is outside the triangle
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    float t = f * edge2.DotProduct(q);

    // intersection point is behind the ray origin
    if (t > EPSILON) {
        // point found
        tr->end = tr->origin + tr->direction * t;
        return true;
    }

    return false;
}

void Trace<Triangle>::TraceLine(Engine* engine,std::vector<Triangle>& allTri) {
	collided = false;
	end = origin;

    float flClose = 99999.99f;

	for (Triangle& tri : allTri) {
        if (intersectTriangle(this, tri))
        {
            float dst = ((Vector3)(origin - tri.findTriangleCenter())).Length();
            if (dst < flClose) {
                collided = true;
                hit = tri;
                flClose = dst;
            }
        }
	}
}