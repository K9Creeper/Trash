#include "world.h"

bool World::AddEngineObject(const std::string& meshName, const EngineObject& go) {
	if (EngineObjects.find(meshName) != EngineObjects.end())
		return false;
	EngineObjects[meshName] = go;

	for (int i = 0; i < EngineObjects[meshName].worldmesh.triangles.size(); i++)
	{
		EngineObjects[meshName].worldmesh.triangles[i].id = std::hash< std::string>{}(meshName)+i;
		AllTriangle.push_back(EngineObjects[meshName].worldmesh.triangles[i]);
	}
	return true;
}