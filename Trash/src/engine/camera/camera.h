#pragma once
#include "../math/mesh.h"
#include "../math/matrix.h"

struct ProjectionMatrix {
	int fFovDegrees;
	float fAspectRatio, fNear, fFar;

	Matrix4x4 matrix;

	bool WorldToScreen(const Triangle& in, Triangle& out);

	Matrix4x4& Create();
};

class Camera {
public:
	void Create(const float& fov);
	void CreateCameraMatrix();

	ProjectionMatrix matProj;
	Matrix4x4 matView;

	Vector3 origin = Vector3(0.f, 1.f, 0.f);
	Vector3 lookDir;

	struct Rotation {
		float yaw = 0.f, 
			pitch = 0.f;

		void ClampYaw() {
			// THIS NEEDS TO BE WORKED ON
			// I am not sure what exactly is 1/2 of a full rotation
			if (this->yaw > 3.1f)
				this->yaw = -3.1f;
			else if (this->yaw < -3.1f)
				this->yaw = 3.1f;
		}
		void ClampPitch() {
			if (this->pitch > 1.5f)
				this->pitch = 1.5f;
			else if (this->pitch < -1.5f)
				this->pitch = -1.5f;
		}
	}rotation;
	
};