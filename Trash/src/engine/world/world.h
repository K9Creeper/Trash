#pragma once
#include <unordered_map>

#include "../EngineObject/EngineObject.h"

struct LightSource {
	Vector3 origin;
	float strenghRad;

};

class World {
public:

private:
	std::unordered_map<std::string, EngineObject>EngineObjects{};

public:
	void AddEngineObject(const std::string& meshName, const EngineObject& go);
	std::unordered_map<std::string, EngineObject>& getEngineObjects() { return EngineObjects; }
	Matrix4x4 matWorld;
	std::vector<LightSource> lightSources;
	int light_directionx = 0;
	int light_directiony = 1;
	int light_directionz = 0;

};