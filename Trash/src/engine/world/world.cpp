#include "world.h"

bool World::AddEngineObject(const std::string& meshName, const EngineObject& go) {
	if (EngineObjects.find(meshName) == EngineObjects.end())
		return false;
	EngineObjects[meshName] = go;

	for (int i = 0; i < go.worldmesh.ptrTriangles.size(); i++)
	{
		go.worldmesh.ptrTriangles[i]->id = std::hash< std::string>{}(meshName)+i;
		AllTriangle.push_back(go.worldmesh.ptrTriangles[i]);
	}
	return true;
}