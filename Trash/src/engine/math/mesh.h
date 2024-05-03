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
	FloodVector2 t[3]; // u, v, w

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
		// Make sure plane normal is indeed normal
		plane_n.Normalise();

		// Return signed shortest distance from point to plane, plane normal must be normalised
		auto dist = [&](Vector3& p)
		{
			Vector3 n = p;
			n.Normalise();
			return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - plane_n.DotProduct(plane_p));
		};

		// Create two temporary storage arrays to classify points either side of plane
		// If distance sign is positive, point lies on "inside" of plane
		Vector3* inside_points[3];  int nInsidePointCount = 0;
		Vector3* outside_points[3]; int nOutsidePointCount = 0;
		FloodVector2* inside_tex[3]; int nInsideTexCount = 0;
		FloodVector2* outside_tex[3]; int nOutsideTexCount = 0;


		// Get signed distance of each point in triangle to plane
		float d0 = dist(p[0]);
		float d1 = dist(p[1]);
		float d2 = dist(p[2]);

		if (d0 >= 0) { inside_points[nInsidePointCount++] = &p[0]; inside_tex[nInsideTexCount++] = &t[0]; }
		else {
			outside_points[nOutsidePointCount++] = &p[0]; outside_tex[nOutsideTexCount++] = &t[0];
		}
		if (d1 >= 0) {
			inside_points[nInsidePointCount++] = &p[1]; inside_tex[nInsideTexCount++] = &t[1];
		}
		else {
			outside_points[nOutsidePointCount++] = &p[1];  outside_tex[nOutsideTexCount++] = &t[1];
		}
		if (d2 >= 0) {
			inside_points[nInsidePointCount++] = &p[2]; inside_tex[nInsideTexCount++] = &t[2];
		}
		else {
			outside_points[nOutsidePointCount++] = &p[2];  outside_tex[nOutsideTexCount++] = &t[2];
		}

		// Now classify triangle points, and break the input triangle into 
		// smaller output triangles if required. There are four possible
		// outcomes...

		if (nInsidePointCount == 0)
		{
			// All points lie on the outside of plane, so clip whole triangle
			// It ceases to exist

			return 0; // No returned triangles are valid
		}

		if (nInsidePointCount == 3)
		{
			// All points lie on the inside of plane, so do nothing
			// and allow the triangle to simply pass through
			out_tri1 = *this;

			return 1; // Just the one returned original triangle is valid
		}

		if (nInsidePointCount == 1 && nOutsidePointCount == 2)
		{
			// Triangle should be clipped. As two points lie outside
			// the plane, the triangle simply becomes a smaller triangle

			// Copy appearance info to new triangle
			out_tri1.col = col;

			// The inside point is valid, so keep that...
			out_tri1.p[0] = *inside_points[0];
			out_tri1.t[0] = *inside_tex[0];

			// but the two new points are at the locations where the 
			// original sides of the triangle (lines) intersect with the plane
			float t;
			out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
			out_tri1.t[1].x = t * (outside_tex[0]->x - inside_tex[0]->x) + inside_tex[0]->x;
			out_tri1.t[1].y = t * (outside_tex[0]->y - inside_tex[0]->y) + inside_tex[0]->y;

			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1], t);
			out_tri1.t[2].x = t * (outside_tex[1]->x - inside_tex[0]->x) + inside_tex[0]->x;
			out_tri1.t[2].y = t * (outside_tex[1]->y - inside_tex[0]->y) + inside_tex[0]->y;

			return 1; // Return the newly formed single triangle
		}

		if (nInsidePointCount == 2 && nOutsidePointCount == 1)
		{
			// Triangle should be clipped. As two points lie inside the plane,
			// the clipped triangle becomes a "quad". Fortunately, we can
			// represent a quad with two new triangles

			// Copy appearance info to new triangles
			out_tri1.col = col;

			out_tri2.col = col;

			// The first triangle consists of the two inside points and a new
			// point determined by the location where one side of the triangle
			// intersects with the plane
			out_tri1.p[0] = *inside_points[0];
			out_tri1.p[1] = *inside_points[1];
			out_tri1.t[0] = *inside_tex[0];
			out_tri1.t[1] = *inside_tex[1];

			float t;
			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
			out_tri1.t[2].x = t * (outside_tex[0]->x - inside_tex[0]->x) + inside_tex[0]->x;
			out_tri1.t[2].y = t * (outside_tex[0]->y - inside_tex[0]->y) + inside_tex[0]->y;

			// The second triangle is composed of one of he inside points, a
			// new point determined by the intersection of the other side of the 
			// triangle and the plane, and the newly created point above
			out_tri2.p[0] = *inside_points[1];
			out_tri2.t[0] = *inside_tex[1];
			out_tri2.p[1] = out_tri1.p[2];
			out_tri2.t[1] = out_tri1.t[2];
			out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], t);
			out_tri2.t[2].x = t * (outside_tex[0]->x - inside_tex[1]->x) + inside_tex[1]->x;
			out_tri2.t[2].y = t * (outside_tex[0]->y - inside_tex[1]->y) + inside_tex[1]->y;
			return 2; // Return two newly formed triangles which form a quad
		}
	}
};

struct Mesh {
	std::vector<Triangle>triangles;

	bool LoadFromObjectFile(std::string sFilename, bool bHasTexture = false)
	{
		std::ifstream f(sFilename);
		if (!f.is_open())
			return false;

		std::vector<Vector3> verts;
		std::vector<FloodVector2> texs;

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
					texs.push_back({v.x, v.y});
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


					triangles.push_back({ {verts[stoi(tokens[0]) - 1], verts[stoi(tokens[2]) - 1], verts[stoi(tokens[4]) - 1]}, {texs[stoi(tokens[1]) - 1], texs[stoi(tokens[3]) - 1], texs[stoi(tokens[5]) - 1]} });

				}

			}

		}
		return true;
	}
};

