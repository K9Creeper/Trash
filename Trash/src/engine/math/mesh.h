#pragma once
#include <vector>
#include "vector.h"
#include "../math/matrix.h"

#include <fstream>
#include <strstream>
#include <algorithm>

struct Triangle {
	Vector3 p[3];

	FloodColor col;

	int ClipAgainstPlane(Vector3 plane_p, Vector3 plane_n, Triangle& out_tri1, Triangle& out_tri2)
	{
		plane_n.Normalise();

		const auto& dist = [&](Vector3 p)
		{
			p.Normalise();
			return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - plane_n.DotProduct(plane_p));
		};

		// Classify points either side of plane
		Vector3* inside_points[3];  int nInsidePointCount = 0;
		Vector3* outside_points[3]; int nOutsidePointCount = 0;

		// Get distance of each point in triangle to plane
		// If dist is positive, point lies on "inside" of plane
		float d0 = dist(p[0]);
		float d1 = dist(p[1]);
		float d2 = dist(p[2]);

		if (d0 >= 0) { inside_points[nInsidePointCount++] = &p[0]; }
		else { outside_points[nOutsidePointCount++] = &p[0]; }
		if (d1 >= 0) { inside_points[nInsidePointCount++] = &p[1]; }
		else { outside_points[nOutsidePointCount++] = &p[1]; }
		if (d2 >= 0) { inside_points[nInsidePointCount++] = &p[2]; }
		else { outside_points[nOutsidePointCount++] = &p[2]; }

		// Now classify triangle points, and break the input triangle into 
		// smaller output triangles if required. There are four possible
		// outcomes...

		if (nInsidePointCount == 0)
		{
			// No good triangles
			return 0;
		}

		if (nInsidePointCount == 3)
		{
			out_tri1 = *this;

			// Return the 1 for original triangle
			return 1;
		}

		if (nInsidePointCount == 1 && nOutsidePointCount == 2)
		{
			// Triangle should be clipped. As two points lie outside
			// the plane, the triangle simply becomes a smaller triangle

			// Copy appearance info to new triangle
			out_tri1.col = col;

			// The inside point is valid, so keep that...
			out_tri1.p[0] = *inside_points[0];

			out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

			// Return the 1 for a new single triangle
			return 1;
		}

		if (nInsidePointCount == 2 && nOutsidePointCount == 1)
		{
			out_tri1.col = col;

			out_tri2.col = col;

			out_tri1.p[0] = *inside_points[0];
			out_tri1.p[1] = *inside_points[1];
			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

			out_tri2.p[0] = *inside_points[1];
			out_tri2.p[1] = out_tri1.p[2];
			out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

			// Return the 2 for two triangles
			return 2;
		}
	}
};

struct Mesh {
	std::vector<Triangle>triangles;

	bool LoadFromObjectFile(std::string sFilename)
	{
		std::ifstream f(sFilename);
		if (!f.is_open())
			return false;

		std::vector<Vector3> verts;

		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			std::strstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				Vector3 v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				triangles.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}
		return true;
	}
};
