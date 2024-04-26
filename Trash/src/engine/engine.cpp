#include "engine.h"
#include "render/render.h"
#include "render/floodgui/flood_gui.h"

#include <thread>

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

		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_A].raw_down) {
			engine->camera.rotation.yaw -= 2.0f * 0.05f;
		}
		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_D].raw_down) {
			engine->camera.rotation.yaw += 2.0f * 0.05f;
		}

		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_UpArrow].raw_down) {
			engine->camera.rotation.pitch -= 2.0f * 0.05f;
		}
		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_DownArrow].raw_down) {
			engine->camera.rotation.pitch += 2.0f * 0.05f;
		}
		Sleep(16);
	}
}

void Engine::Start() {

	meshCube.LoadFromObjectFile("mountains.obj");
	
	render->Init();
	camera.Create(90.f);

	std::thread it(InputThread, this);

	groundColor = FloodColor(255, 255, 255, 255);

	render->Begin([&]() {
		OnRender();
	});
	running = false;
	it.join();
}

constexpr bool pointOnScreen(const float& x, const float& y)
{
	return (x > 0 && x < FloodGui::Context.Display.DisplaySize.x&& y > 0 && FloodGui::Context.Display.DisplaySize.y);
}

void shadeColor(FloodColor& col, const float& perc)
{
	col.r() *= perc;
	col.b() *= perc;
	col.g() *= perc;
}

void Engine::OnRender() {
	const float& height = FloodGui::Context.Display.DisplaySize.y;
	const float& width = FloodGui::Context.Display.DisplaySize.x;

	// Create World Matrix and apply needed translations
	{
		matWorld.MakeIdentity();

		Matrix4x4 matTrans; matTrans.MakeTranslation(0.0f, 0.0f, 0.0f);

		matWorld.MultiplyMatrix(matWorld, matTrans);
	}

	// Setup/Update camera and view projection
	{
		camera.CreateCameraMatrix();
		camera.matProj.Create();
	}

	static std::vector<Triangle> clipedTriangles{};
	clipedTriangles.clear();

	for (const Triangle& tri : meshCube.triangles)
	{
		Triangle triProjected, triTransformed, triViewed;

		triTransformed.p[0] = matWorld.MultiplyVector(tri.p[0]);
		triTransformed.p[1] = matWorld.MultiplyVector(tri.p[1]);
		triTransformed.p[2] = matWorld.MultiplyVector(tri.p[2]);
		
		Vector3 line1 = (triTransformed.p[1] - triTransformed.p[0]);
		const Vector3& line2 = (triTransformed.p[2] - triTransformed.p[0]);
		const Vector3& normal = (line1.CrossProduct(line2)).Normalise();

		if (normal.DotProduct((triTransformed.p[0] - camera.origin)) >= 0.0f)
			continue;
			
		Vector3 light_direction = { 0.0f, 1.0f, 1.0f };
		light_direction.Normalise();

		float dp = max(0.1f, light_direction.DotProduct(normal));

		triViewed.p[0] = camera.matView.MultiplyVector(triTransformed.p[0]);
		triViewed.p[1] = camera.matView.MultiplyVector(triTransformed.p[1]);
		triViewed.p[2] = camera.matView.MultiplyVector(triTransformed.p[2]);

		FloodColor col = groundColor;
		shadeColor(col, dp);
		triViewed.col = col;


		static Triangle clipped[2];
		int nClippedTriangles = triViewed.ClipAgainstPlane(Vector3{ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, clipped[0], clipped[1]);
			
		for (int n = 0; n < nClippedTriangles; n++)
		{
			if (!camera.matProj.WorldToScreen(clipped[n], triProjected))
				continue;
				
			triProjected.col = clipped[n].col;

			triProjected.p[0].x *= -1.0f;
			triProjected.p[1].x *= -1.0f;
			triProjected.p[2].x *= -1.0f;
			triProjected.p[0].y *= -1.0f;
			triProjected.p[1].y *= -1.0f;
			triProjected.p[2].y *= -1.0f;

			// Offset verts into visible normalised space
			static const Vector3& vOffsetView = { 1,1,0 };
			triProjected.p[0] = (triProjected.p[0] + vOffsetView);
			triProjected.p[1] = (triProjected.p[1] + vOffsetView);
			triProjected.p[2] = (triProjected.p[2] + vOffsetView);
			triProjected.p[0].x *= 0.5f * (float)width;
			triProjected.p[0].y *= 0.5f * (float)height;
			triProjected.p[1].x *= 0.5f * (float)width;
			triProjected.p[1].y *= 0.5f * (float)height;
			triProjected.p[2].x *= 0.5f * (float)width;
			triProjected.p[2].y *= 0.5f * (float)height;

			if (!(pointOnScreen(triProjected.p[0].x, triProjected.p[0].y) || pointOnScreen(triProjected.p[1].x, triProjected.p[1].y) || pointOnScreen(triProjected.p[2].x, triProjected.p[2].y))) 
				continue;
					
			clipedTriangles.push_back(triProjected);
		}
	}
	
	sort(clipedTriangles.begin(), clipedTriangles.end(), [](Triangle& t1, Triangle& t2) { return ((t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f) > ((t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f); });

	for (const auto& triToRaster : clipedTriangles)
	{
		Triangle clipped[2];
		std::list<Triangle> listTriangles;

		// Add initial triangle
		listTriangles.push_back(triToRaster);
		int nNewTriangles = 1;

		for (int p = 0; p < 4; p++)
		{
			int nTrisToAdd = 0;
			while (nNewTriangles > 0)
			{
				Triangle test = listTriangles.front();
				listTriangles.pop_front();
				nNewTriangles--;

				switch (p)
				{
					case 0:	nTrisToAdd = test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, clipped[0], clipped[1]); break;
					case 1:	nTrisToAdd = test.ClipAgainstPlane({ 0.0f, (float)height - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, clipped[0], clipped[1]); break;
					case 2:	nTrisToAdd = test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, clipped[0], clipped[1]); break;
					case 3:	nTrisToAdd = test.ClipAgainstPlane({ (float)width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, clipped[0], clipped[1]); break;
				}

				for (int w = 0; w < nTrisToAdd; w++)
					listTriangles.push_back(clipped[w]);
			}
			nNewTriangles = listTriangles.size();
		}
		for (const Triangle& tri : listTriangles)
		{
			FloodGui::Context.GetBackgroundDrawList()->AddTriangleFilled({ tri.p[0].x, tri.p[0].y }, { tri.p[1].x, tri.p[1].y }, { tri.p[2].x, tri.p[2].y }, tri.col);
		}
	}

	
	{
		// FloodGui has a slight issue
		// Need to update FloodGui later!
		FloodGui::BeginWindow("Camera Editer");
		
		FloodGui::Color3Slider("Ground Color", groundColor.data());

		FloodGui::EndWindow();
	}
	
}