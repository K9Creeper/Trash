#pragma once
#include <cmath>
#include "../render/floodgui/flood_gui_math.h"

class Vector3 : public FloodVector3 {
public:
	Vector3() { x = 0; y = 0; z = 0; }
	Vector3(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }

	Vector3(const FloodVector3& v){ x = v.x; y = v.y; z = v.z; }

	constexpr float DotProduct(const Vector3& v2) const
	{
		return x * v2.x + y * v2.y + z * v2.z;
	}

	float Length() const
	{
		return sqrtf(DotProduct(*this));
	}

	Vector3& Normalise()
	{
		const float& l = Length();
		x = x / l;
		y = y / l;
		z = z / l;
		return *this;
	}

	Vector3 CrossProduct(const Vector3& v2) const
	{
		Vector3 v;
		v.x = y * v2.z - z * v2.y;
		v.y = z * v2.x - x * v2.z;
		v.z = x * v2.y - y * v2.x;
		return v;
	}
};

inline Vector3 Vector_IntersectPlane(Vector3& plane_p, Vector3& plane_n, Vector3& lineStart, Vector3& lineEnd)
{
	plane_n.Normalise();
	const float& plane_d = -plane_n.DotProduct(plane_p);
	const float& ad = lineStart.DotProduct(plane_n);
	const float& bd = lineEnd.DotProduct(plane_n);
	const float& t = (-plane_d - ad) / (bd - ad);
	Vector3 lineStartToEnd = (lineEnd - lineStart);
	Vector3 lineToIntersect = (lineStartToEnd * t);
	return (lineStart + lineToIntersect);
}