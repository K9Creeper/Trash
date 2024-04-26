#pragma once
#include <unordered_map>

#include "../gameobject/gameobject.h"

class World {
public:

private:
	std::unordered_map<std::string, GameObject>GameObjects{};

public:
	void AddGameObject(const std::string& meshName, const GameObject& go);
	std::unordered_map<std::string, GameObject>& getGameObjects() { return GameObjects; }
	Matrix4x4 matWorld;
	int light_directionx = 0;
	int light_directiony = 1;
	int light_directionz = 0;

};