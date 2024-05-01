#include "camera.h"
#include "../render/floodgui/flood_gui.h"

Matrix4x4& ProjectionMatrix::Create() {
	float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
	matrix.m[0][0] = fAspectRatio * fFovRad;
	matrix.m[1][1] = fFovRad;
	matrix.m[2][2] = fFar / (fFar - fNear);
	matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}

bool ProjectionMatrix::WorldToScreen(const Triangle& in, Triangle& out) {

	static float w;

	out.p[0] = matrix.MultiplyVectorW(in.p[0], w);
	if (w < 0.001f) return false;

	out.p[1] = matrix.MultiplyVectorW(in.p[1], w);
	if (w < 0.001f) return false;

	out.p[2] = matrix.MultiplyVectorW(in.p[2], w);
	if (w < 0.001f) return false;

	out.t[0] = in.t[0];
	out.t[1] = in.t[1];
	out.t[2] = in.t[2];

	return true;
}

void Camera::Create(const float& fov) {
	{
		const float& height = FloodGui::Context.Display.DisplaySize.y;
		const float& width = FloodGui::Context.Display.DisplaySize.x;

		matProj.fNear = 0.1f;
		matProj.fFar = 1000.0f;
		matProj.fFovDegrees = fov;
		matProj.fAspectRatio = (float)height / (float)width;
	}
	matProj.Create();
}

void Camera::CreateCameraMatrix()
{
	Vector3 vUp = { 0,1,0 };
	Vector3 vTarget = { 0,0,1 };

	rotation.ClampPitch();
	rotation.ClampYaw();

	Matrix4x4 matCameraRoty; matCameraRoty.MakeRotationY(rotation.yaw);
	Matrix4x4 matCameraRotx; matCameraRotx.MakeRotationX(rotation.pitch);

	lookDir = matCameraRotx.MultiplyVector(vTarget);
	lookDir = matCameraRoty.MultiplyVector(lookDir);

	vTarget = (origin + lookDir);
	Matrix4x4 matCamera; matCamera.PointAt(origin, vTarget, vUp);

	matView.QuickInverse(matCamera);
}