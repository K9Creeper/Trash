#pragma once
#include <vector>
#include "vector.h"
#include "../math/matrix.h"

#include <fstream>
#include <strstream>
#include <algorithm>
#include <string>

struct Triangle {

	Vector3 p[3];
	Vector3 t[3];

	FloodColor col;

	float z;

	uint64_t id;

	Vector3 findTriangleCenter() {
		Vector3 center;
		center.x = (p[0].x + p[1].x + p[2].x) / 3.0f;
		center.y = (p[0].y + p[1].y + p[2].y) / 3.0f;
		center.z = (p[0].z + p[1].z + p[2].z) / 3.0f;
		return center;
	}

	constexpr bool operator==(const Triangle& tri)const
	{
		return tri.id == id;
	}

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

	std::vector<Triangle*>ptrTriangles;

	void popPtrTri() {
		ptrTriangles.reserve(triangles.size());
		for (int i = 0; i < triangles.size(); i++)
			ptrTriangles.push_back(&triangles[i]);
	}

	bool LoadFromObjectFile(std::string sFilename, bool bHasTexture = false)
	{
		std::ifstream f(sFilename);
		if (!f.is_open())
			return false;

		std::vector<Vector3> verts;
		std::vector<Vector3> texs;

		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			std::strstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				if (line[1] == 't')
				{
					FloodVector2 v;
					s >> junk >> junk >> v.x >> v.y;
					// A little hack for the spyro texture
					//v.u = 1.0f - v.u;
					//v.v = 1.0f - v.v;
					texs.push_back({v.x, v.y, 1.f});
				}
				else
				{
					Vector3 v;
					s >> junk >> v.x >> v.y >> v.z;
					verts.push_back(v);
				}
			}

			if (!bHasTexture)
			{
				if (line[0] == 'f')
				{
					int f[3];
					s >> junk >> f[0] >> f[1] >> f[2];
					triangles.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
				}
			}
			else
			{
				if (line[0] == 'f')
				{
					s >> junk;

					std::string tokens[6];
					int nTokenCount = -1;


					while (!s.eof())
					{
						char c = s.get();
						if (c == ' ' || c == '/')
							nTokenCount++;
						else
							tokens[nTokenCount].append(1, c);
					}

					tokens[nTokenCount].pop_back();


					triangles.push_back({ verts[stoi(tokens[0]) - 1], verts[stoi(tokens[2]) - 1], verts[stoi(tokens[4]) - 1],
						texs[stoi(tokens[1]) - 1], texs[stoi(tokens[3]) - 1], texs[stoi(tokens[5]) - 1] });

				}

			}

		}
		return true;
	}
};

