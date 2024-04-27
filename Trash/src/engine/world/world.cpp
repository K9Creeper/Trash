#include "world.h"

void World::AddEngineObject(const std::string& meshName, const EngineObject& go) {
	EngineObjects[meshName] = go;
}