#include "engine.h"
#include "render/render.h"
#include "render/floodgui/flood_gui.h"

#include <thread>

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

Engine::Engine() {
	render = new Render();
}

void InputThread(Engine* engine)
{
	while (engine->running)
	{
		Vector3 vForward = (engine->camera.lookDir * 8.0f * 0.05f);

		// Standard FPS Control scheme, but turn instead of strafe
		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_W].raw_down)
			engine->camera.origin = (engine->camera.origin + vForward);

		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_S].raw_down)
			engine->camera.origin = (engine->camera.origin - vForward);

		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_A].raw_down)
			engine->camera.fYaw -= 2.0f * 0.05f;

		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_D].raw_down)
			engine->camera.fYaw += 2.0f * 0.05f;
		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_UpArrow].raw_down)
			engine->camera.fPitch += 2.0f * 0.05f;
		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_DownArrow].raw_down)
			engine->camera.fPitch -= 2.0f * 0.05f;
		Sleep(16);
	}
}

void Engine::Start() {

	meshCube.LoadFromObjectFile("mountains.obj");
	
	render->Init();
	camera.Create(90.f);

	std::thread it(InputThread, this);

	render->Begin([&]() {
		OnRender();
	});
	running = false;
	it.join();
}

void Camera::CreateCameraMatrix()
{
	Vector3 vUp = { 0,1,0 };
	Vector3 vTarget = { 0,0,1 };
	Matrix4x4 matCameraRoty; matCameraRoty.MakeRotationY(fYaw);
	Matrix4x4 matCameraRotx; matCameraRotx.MakeRotationX(fPitch);

	float w;
	lookDir = matCameraRotx.MultiplyVector(vTarget, w);
	lookDir = matCameraRoty.MultiplyVector(lookDir, w);

	vTarget = (origin + lookDir);
	Matrix4x4 matCamera; matCamera.PointAt(origin, vTarget, vUp);

	matView.QuickInverse(matCamera);
}

constexpr bool pointOnScreen(const float& x, const float& y)
{
	return (x > 0 && x < FloodGui::Context.Display.DisplaySize.x&& y > 0 && FloodGui::Context.Display.DisplaySize.y);
}

void Engine::OnRender() {
	const float& height = FloodGui::Context.Display.DisplaySize.y;
	const float& width = FloodGui::Context.Display.DisplaySize.x;

	Matrix4x4 matRotZ, matRotX;
	matRotZ.MakeRotationZ(0);
	matRotX.MakeRotationX(0);

	Matrix4x4 matTrans;
	matTrans.MakeTranslation(0.0f, 0.0f, 5.0f);

	Matrix4x4 matWorld;
	matWorld.MakeIdentity();					// Form World Matrix
	matWorld.MultiplyMatrix(matRotZ, matRotX);	// Transform by rotation
	matWorld.MultiplyMatrix(matWorld, matTrans); // Transform by translation


	camera.CreateCameraMatrix();
	camera.matProj.Create();

	for (Triangle& tri : meshCube.triangles)
	{
		Triangle triProjected, triTransformed, triViewed;

		float w;
		triTransformed.p[0] = matWorld.MultiplyVector( tri.p[0], w);
		triTransformed.p[1] = matWorld.MultiplyVector(tri.p[1], w);
		triTransformed.p[2] = matWorld.MultiplyVector(tri.p[2], w);
		
		Vector3 normal, line1, line2;

		line1 = (triTransformed.p[1]- triTransformed.p[0]);
		line2 = (triTransformed.p[2]- triTransformed.p[0]);

		normal = Vector_Normalise(Vector_CrossProduct(line1, line2));

		if (Vector_DotProduct(normal, (triTransformed.p[0] - camera.origin)) < 0.0f) {

			Vector3 light_direction = { 0.0f, 1.0f, -1.0f };
			light_direction = Vector_Normalise(light_direction);

			float dp = 255.f *  max(0.1f, Vector_DotProduct(light_direction, normal));

			triViewed.p[0] = camera.matView.MultiplyVector(triTransformed.p[0], w);
			triViewed.p[1] = camera.matView.MultiplyVector(triTransformed.p[1], w);
			triViewed.p[2] = camera.matView.MultiplyVector(triTransformed.p[2], w);

			triProjected.p[0] = camera.matProj.matrix.MultiplyVector(triViewed.p[0], w);
			if (w < 0) continue;
			triProjected.p[1] = camera.matProj.matrix.MultiplyVector(triViewed.p[1], w);
			if (w < 0) continue;
			triProjected.p[2] = camera.matProj.matrix.MultiplyVector(triViewed.p[2], w);
			if (w < 0) continue;

			triProjected.p[0].x *= -1.0f;
			triProjected.p[1].x *= -1.0f;
			triProjected.p[2].x *= -1.0f;
			triProjected.p[0].y *= -1.0f;
			triProjected.p[1].y *= -1.0f;
			triProjected.p[2].y *= -1.0f;

			// Offset verts into visible normalised space
			Vector3 vOffsetView = { 1,1,0 };
			triProjected.p[0] = (triProjected.p[0] + vOffsetView);
			triProjected.p[1] = (triProjected.p[1] + vOffsetView);
			triProjected.p[2] = (triProjected.p[2] + vOffsetView);
			triProjected.p[0].x *= 0.5f * (float)width;
			triProjected.p[0].y *= 0.5f * (float)height;
			triProjected.p[1].x *= 0.5f * (float)width;
			triProjected.p[1].y *= 0.5f * (float)height;
			triProjected.p[2].x *= 0.5f * (float)width;
			triProjected.p[2].y *= 0.5f * (float)height;

			if(pointOnScreen(triProjected.p[0].x, triProjected.p[0].y) || pointOnScreen(triProjected.p[1].x, triProjected.p[1].y) || pointOnScreen(triProjected.p[2].x, triProjected.p[2].y))
				render->DrawTri(triProjected.p[0].x, triProjected.p[0].y,
					triProjected.p[1].x, triProjected.p[1].y,
					triProjected.p[2].x, triProjected.p[2].y);
		}
	}


	/*
	{
		FloodGui::BeginWindow("Camera Editer");
		
		FloodGui::IntSlider("FOV", &(camera.matProj.fFovDegrees), 10, 180);

		FloodGui::EndWindow();
	}
	*/

}