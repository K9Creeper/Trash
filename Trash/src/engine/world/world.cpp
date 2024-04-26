#include "world.h"

void World::AddGameObject(const std::string& meshName, const GameObject& go) {
	GameObjects[meshName] = go;
}