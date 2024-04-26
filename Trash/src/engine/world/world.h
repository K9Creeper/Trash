#pragma once
#include <unordered_map>

#include "../math/mesh.h"

class World {
public:

private:
	std::unordered_map<std::string, Mesh>meshes{};

public:
	void AddMesh(const std::string& meshName, const Mesh& mesh);
	std::unordered_map<std::string, Mesh>& getMeshes() { return meshes; }
	Matrix4x4 matWorld;
	int light_directionx = 0;
	int light_directiony = 1;
	int light_directionz = 0;

};