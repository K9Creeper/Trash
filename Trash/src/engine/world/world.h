#pragma once
#include <unordered_map>
#include <vector>

#include "../EngineObject/EngineObject.h"

struct LightSource {
public:
	Vector3 origin;
	float strenghRad;
};

class World {
public:

private:
	std::unordered_map<std::string, EngineObject>EngineObjects{};
	std::vector<Triangle>AllTriangle{};

public:
	bool AddEngineObject(const std::string& meshName, const EngineObject& go);
	std::unordered_map<std::string, EngineObject>& getEngineObjects() { return EngineObjects; }
	std::vector<Triangle>& getAllTriangles() { return AllTriangle; }
	Matrix4x4 matWorld;
	std::vector<LightSource> lightSources;
};