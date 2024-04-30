#include "world.h"

void World::AddEngineObject(const std::string& meshName, const EngineObject& go) {
	EngineObjects[meshName] = go;

	AllTriangle.clear();
	for (auto& [name, obj] : EngineObjects) {
		for(int i = 0; i < obj.worldmesh.triangles.size(); i++)
			AllTriangle.push_back(&obj.worldmesh.triangles[i]);
	}
}