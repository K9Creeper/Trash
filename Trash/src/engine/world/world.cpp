#include "world.h"

void World::AddMesh(const std::string& meshName, const Mesh& mesh) {
	meshes[meshName] = mesh;
}