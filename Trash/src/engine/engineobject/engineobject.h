#pragma once

#include "../math/mesh.h"

class EngineObject {
public:
	EngineObject(){}
	EngineObject(const Mesh& mesh, const Vector3& origin, const float& scale = 1.f) {
		this->omesh = mesh;
		this->origin = origin;

		for (Triangle& t : omesh.triangles) {
			Triangle tt;
			tt.p[0] = t.p[0] + origin; 
			tt.p[1] = t.p[1] + origin; 
			tt.p[2] = t.p[2] + origin;

			tt.p[0] = tt.p[0] * scale;
			tt.p[1] = tt.p[1] * scale;
			tt.p[2] = tt.p[2] * scale;

			tt.t[0] = t.t[0];
			tt.t[1] = t.t[1];
			tt.t[2] = t.t[2];
			
			worldmesh.triangles.push_back(tt);
		}
	}
private:
	Mesh omesh;
	std::string meshName;
public:
	Vector3 origin;
	Mesh worldmesh;
};