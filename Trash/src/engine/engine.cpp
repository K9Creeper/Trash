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
	
	if(Mesh cube; cube.LoadFromObjectFile("teapot.obj"))
		world.AddGameObject("teapot", GameObject(cube, {0.f, 0.f, 0.f}));

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
	return (x >= 0 && x < FloodGui::Context.Display.DisplaySize.x&& y >= 0 && y < FloodGui::Context.Display.DisplaySize.y);
}

void shadeColor(FloodColor& col, const float& perc)
{
	col.r() *= perc;
	col.b() *= perc;
	col.g() *= perc;
}

void DrawPixel(const float& x, const float& y, FloodColor col) {
	FloodGui::Context.GetBackgroundDrawList()->AddRectFilled({ x, y }, { x + 1, y + 1 }, col);
}

void TexturedTriangle(float* pDepthBuffer, const Triangle& tri)
{
	const int& height = FloodGui::Context.Display.DisplaySize.y;
	const int& width = FloodGui::Context.Display.DisplaySize.x;

	
}

float area(int x1, int y1, int x2, int y2, int x3, int y3)
{
	return abs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
}

bool isInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
{
	float A = area(x1, y1, x2, y2, x3, y3);

	float A1 = area(x, y, x2, y2, x3, y3);

	float A2 = area(x1, y1, x, y, x3, y3);

	float A3 = area(x1, y1, x2, y2, x, y);

	return (A == A1 + A2 + A3);
}

void Engine::OnRender() {
	const float& height = FloodGui::Context.Display.DisplaySize.y;
	const float& width = FloodGui::Context.Display.DisplaySize.x;

	// Create World Matrix and apply needed translations
	{
		world.matWorld.MakeIdentity();

		Matrix4x4 matTrans; matTrans.MakeTranslation(0.0f, 0.0f, 0.0f);

		world.matWorld.MultiplyMatrix(world.matWorld, matTrans);
	}

	// Setup/Update camera and view projection
	{
		camera.CreateCameraMatrix();
		camera.matProj.Create();
	}

	struct Clipped {
		Triangle clipped[2];
		int nClippedTriangles = 0;

		bool	valid = false;
		Triangle finish;
	};

	static std::list<Triangle> listlistTriangles;
	std::vector< Clipped>clips;
	for (const auto& [name, Objects] : world.getGameObjects()) {
		for (const Triangle& tri : Objects.worldmesh.triangles)
		{
			Triangle triTransformed, triViewed;

			triTransformed.p[0] = world.matWorld.MultiplyVector(tri.p[0]);
			triTransformed.p[1] = world.matWorld.MultiplyVector(tri.p[1]);
			triTransformed.p[2] = world.matWorld.MultiplyVector(tri.p[2]);
			triTransformed.t[0] = tri.t[0];
			triTransformed.t[1] = tri.t[1];
			triTransformed.t[2] = tri.t[2];

			Vector3 line1 = (triTransformed.p[1] - triTransformed.p[0]);
			const Vector3& line2 = (triTransformed.p[2] - triTransformed.p[0]);
			const Vector3& normal = (line1.CrossProduct(line2)).Normalise();

			if (normal.DotProduct((triTransformed.p[0] - camera.origin)) > 0.0f)
				continue;

			Vector3 light_direction = { (float)world.light_directionx, (float)world.light_directiony , (float)world.light_directionz };

			triViewed.p[0] = camera.matView.MultiplyVector(triTransformed.p[0]);
			triViewed.p[1] = camera.matView.MultiplyVector(triTransformed.p[1]);
			triViewed.p[2] = camera.matView.MultiplyVector(triTransformed.p[2]);
			triViewed.t[0] = triTransformed.t[0];
			triViewed.t[1] = triTransformed.t[1];
			triViewed.t[2] = triTransformed.t[2];

			FloodColor col = FloodColor(255, 255, 255, 255);
			shadeColor(col, max(0.1f, light_direction.Normalise().DotProduct(normal)));
			triViewed.col = col;

			Clipped clip;
			clip.nClippedTriangles = triViewed.ClipAgainstPlane(Vector3{ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, clip.clipped[0], clip.clipped[1]);
			
			for (int n = 0; n < clip.nClippedTriangles; n++)
			{
				if (!camera.matProj.WorldToScreen(clip.clipped[n], clip.finish))
					continue;

				clip.finish.col = clip.clipped[n].col;

				clip.finish.t[0] = clip.clipped[n].t[0];
				clip.finish.t[1] = clip.clipped[n].t[1];
				clip.finish.t[2] = clip.clipped[n].t[2];


				clip.finish.t[0].x = clip.finish.t[0].x / clip.finish.p[0].z;
				clip.finish.t[1].x = clip.finish.t[1].x / clip.finish.p[1].z;
				clip.finish.t[2].x = clip.finish.t[2].x / clip.finish.p[2].z;

				clip.finish.t[0].y = clip.finish.t[0].y / clip.finish.p[0].z;
				clip.finish.t[1].y = clip.finish.t[1].y / clip.finish.p[1].z;
				clip.finish.t[2].y = clip.finish.t[2].y / clip.finish.p[2].z;

				clip.finish.t[0].z = 1.0f / clip.finish.p[0].z;
				clip.finish.t[1].z = 1.0f / clip.finish.p[1].z;
				clip.finish.t[2].z = 1.0f / clip.finish.p[2].z;


				clip.finish.p[0].x *= -1.0f;
				clip.finish.p[1].x *= -1.0f;
				clip.finish.p[2].x *= -1.0f;
				clip.finish.p[0].y *= -1.0f;
				clip.finish.p[1].y *= -1.0f;
				clip.finish.p[2].y *= -1.0f;

				// Offset verts into visible normalised space
				static const Vector3& vOffsetView = { 1,1,0 };
				clip.finish.p[0] = (clip.finish.p[0] + vOffsetView);
				clip.finish.p[1] = (clip.finish.p[1] + vOffsetView);
				clip.finish.p[2] = (clip.finish.p[2] + vOffsetView);
				clip.finish.p[0].x *= 0.5f * (float)width;
				clip.finish.p[0].y *= 0.5f * (float)height;
				clip.finish.p[1].x *= 0.5f * (float)width;
				clip.finish.p[1].y *= 0.5f * (float)height;
				clip.finish.p[2].x *= 0.5f * (float)width;
				clip.finish.p[2].y *= 0.5f * (float)height;


				clip.valid = (pointOnScreen(clip.finish.p[1].x, clip.finish.p[1].y) || pointOnScreen(clip.finish.p[2].x, clip.finish.p[2].y) || pointOnScreen(clip.finish.p[0].x, clip.finish.p[0].y));
			
				if (!clip.valid)
					continue;

				Triangle clipped[2];
				std::list<Triangle> listTriangles;

				listTriangles.push_back(clip.finish);
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
				for(const Triangle& t : listTriangles)
					listlistTriangles.push_back(t);
			}
		}
	}

	/* 
		MUST KEEP THIS
	*/

	listlistTriangles.sort([](Triangle& t1, Triangle& t2) { return (t1.z) > (t2.z); });

	for (const Triangle& tri : listlistTriangles)
	{
		FloodGui::Context.GetBackgroundDrawList()->AddTriangleFilled({ tri.p[0].x, tri.p[0].y }, { tri.p[1].x, tri.p[1].y }, { tri.p[2].x, tri.p[2].y }, tri.col);
	}

	listlistTriangles.clear();

	{
		// FloodGui has a slight issue
		// Need to update FloodGui later!
		FloodGui::BeginWindow("Camera Editor");
		
		FloodGui::IntSlider("Lighting X", &world.light_directionx, -10, 10);
		FloodGui::IntSlider("Lighting Y", &world.light_directiony, -10, 10);
		FloodGui::IntSlider("Lighting Z", &world.light_directionz, -10, 10);

		FloodGui::EndWindow();
	}
	
}