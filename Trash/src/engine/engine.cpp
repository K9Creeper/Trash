#include "engine.h"
#include "render/render.h"
#include "render/floodgui/flood_gui.h"

#include "trace/trace.h"

#include <thread>
#include <mutex>


bool raytracing = false;

Engine::Engine() {
	render = new Render();
}

void InputThread(Engine* engine)
{
	while (engine->running)
	{
		static Vector3 vForward;

		{
			// Use camera math to simulate a non-pitch camera direction
			// we can use this to like do cool things
			Vector3 vTarget = { 0,0,1 };

			Matrix4x4 matCameraRoty; matCameraRoty.MakeRotationY(engine->camera.rotation.yaw);

			vForward = matCameraRoty.MultiplyVector(vTarget) * 8.f * 0.15f;
		}
		Vector3 vRight = Vector3(vForward.z, 0, -vForward.x);

		// Standard FPS Control scheme, but turn instead of strafe
		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_W].raw_down)
			engine->camera.origin = (engine->camera.origin + Vector3(vForward.x, 0, vForward.z));

		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_S].raw_down)
			engine->camera.origin = (engine->camera.origin - Vector3(vForward.x, 0, vForward.z));

		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_A].raw_down) {
			engine->camera.origin = (engine->camera.origin + vRight);
		}
		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_D].raw_down) {
			engine->camera.origin = (engine->camera.origin - vRight);
		}

		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_Q].raw_down) {
			engine->camera.origin.y += 2.0f;
		}
		if (FloodGui::Context.IO.KeyboardInputs[FloodGuiKey_E].raw_down) {
			engine->camera.origin.y -= 2.0f;
		}

		static POINT p;
		if (engine->render->getWindow()->_hwnd == GetForegroundWindow()) {
			GetCursorPos(&p);
			FloodVector2 diff = FloodVector2(FloodGui::Context.Display.DisplayPosition.x + FloodGui::Context.Display.DisplaySize.x / 2.f, FloodGui::Context.Display.DisplayPosition.y + FloodGui::Context.Display.DisplaySize.y / 2.f) - FloodVector2(p.x, p.y);
			diff.y -= 0.5; // For some reason it always seems to be .5 at all times??
			engine->camera.rotation.pitch -= diff.y * .015f;
			engine->camera.rotation.yaw -= diff.x * .015f;

			if (engine->render->getWindow()->lockMouse)
				SetCursorPos(FloodGui::Context.Display.DisplayPosition.x + FloodGui::Context.Display.DisplaySize.x / 2.f, FloodGui::Context.Display.DisplayPosition.y + FloodGui::Context.Display.DisplaySize.y / 2.f);
		}
		Sleep(16);
	}
}

void Engine::Start() {
	Mesh cube;
	if (cube.LoadFromObjectFile("mountains.obj")) {
		world.AddEngineObject("ground", EngineObject(cube, { 0.f, 0.f, 0.f }));
	}
	
	world.lightSources.push_back({ {0, 100, 0}, 999.9f});

	render->Init();

	camera.Create(90.f);

	std::thread it(InputThread, this);

	groundColor = FloodColor(255, 205, 255, 255);

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

void ProcessTriangles(Engine* engine, std::vector<Triangle>* triangles, std::vector<Triangle>* listlistTriangles, std::mutex* mutex)
{
	const float& height = FloodGui::Context.Display.DisplaySize.y;
	const float& width = FloodGui::Context.Display.DisplaySize.x;

	struct Clipped {
		Triangle clipped[2];
		int nClippedTriangles = 0;

		bool	valid = false;
		Triangle finish;
	};

	std::vector<Triangle>& allTris = engine->world.getAllTriangles();

	std::list<Triangle>list;
	for (int jj = 0; jj < triangles->size(); jj++) {
		Triangle& tri = (*triangles)[jj];
		Triangle triTransformed, triViewed;

		// Transform triangle vertices
		for (int i = 0; i < 3; ++i) {
			triTransformed.p[i] = engine->world.matWorld.MultiplyVector(tri.p[i]);
			triTransformed.t[i] = tri.t[i];
			triTransformed.col = tri.col;
		}

		// triangle normal
		Vector3 line1 = triTransformed.p[1] - triTransformed.p[0];
		Vector3 line2 = triTransformed.p[2] - triTransformed.p[0];
		Vector3 normal = line1.CrossProduct(line2).Normalise();

		float n = normal.DotProduct(triTransformed.p[0] - engine->camera.origin);

		// Check backface culling
		if (n > 0.0f)
			continue;

		// triangle to camera view
		for (int i = 0; i < 3; ++i) {
			triViewed.p[i] = engine->camera.matView.MultiplyVector(triTransformed.p[i]);
			triViewed.t[i] = triTransformed.t[i];
			triViewed.col = triTransformed.col;
		}

		// shading / shadowsish
		for (LightSource& ls : engine->world.lightSources) {
			FloodColor col = FloodColor(255, 255, 255, 255);

			Vector3 direction = ls.origin - tri.findTriangleCenter();
			direction.Normalise();
			
			if (((Vector3)(tri.findTriangleCenter() - ls.origin)).Length() <= ls.strenghRad ) {
				shadeColor(col, max(0.1f, direction.DotProduct(normal)));

				if (col.r() > triViewed.col.r() && col.g() > triViewed.col.b() && col.g() > triViewed.col.g())
				{
					triViewed.col = col;
				}
			}
		}

		// Clip triangle against near plane
		Clipped clip;
		clip.nClippedTriangles = triViewed.ClipAgainstPlane(Vector3{ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, clip.clipped[0], clip.clipped[1]);

		// Project and transform clipped triangles
		for (int n = 0; n < clip.nClippedTriangles; ++n) {
			if (!engine->camera.matProj.WorldToScreen(clip.clipped[n], clip.finish))
				continue;

			clip.finish.col = clip.clipped[n].col;

			// Perform viewport transformation
			clip.finish.p[0].x *= -1.0f;
			clip.finish.p[1].x *= -1.0f;
			clip.finish.p[2].x *= -1.0f;
			clip.finish.p[0].y *= -1.0f;
			clip.finish.p[1].y *= -1.0f;
			clip.finish.p[2].y *= -1.0f;

			// Offset verts into visible normalised space
			static Vector3 vOffsetView = { 1,1,0 };
			clip.finish.p[0] = (clip.finish.p[0] + vOffsetView);
			clip.finish.p[1] = (clip.finish.p[1] + vOffsetView);
			clip.finish.p[2] = (clip.finish.p[2] + vOffsetView);
			clip.finish.p[0].x *= 0.5f * (float)width;
			clip.finish.p[0].y *= 0.5f * (float)height;
			clip.finish.p[1].x *= 0.5f * (float)width;
			clip.finish.p[1].y *= 0.5f * (float)height;
			clip.finish.p[2].x *= 0.5f * (float)width;
			clip.finish.p[2].y *= 0.5f * (float)height;
			// Check if triangle is on screen
			clip.valid = pointOnScreen(clip.finish.p[0].x, clip.finish.p[0].y) ||
				pointOnScreen(clip.finish.p[1].x, clip.finish.p[1].y) ||
				pointOnScreen(clip.finish.p[2].x, clip.finish.p[2].y);

			if (!clip.valid)
				continue;

			{
				// Clip against screen edges
				Triangle clipped[2];
				std::list<Triangle> listTriangles;
				clip.finish.z = (clip.finish.p[0].z + clip.finish.p[1].z + clip.finish.p[2].z) / 3.0f;
				listTriangles.push_back(clip.finish);
				int nNewTriangles = 1;

				for (int p = 0; p < 4; ++p) {
					int nTrisToAdd = 0;
					while (nNewTriangles > 0) {
						Triangle test = listTriangles.front();
						listTriangles.pop_front();
						--nNewTriangles;

						switch (p) {
							case 0: nTrisToAdd = test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, clipped[0], clipped[1]); break;
							case 1: nTrisToAdd = test.ClipAgainstPlane({ 0.0f, (float)height - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, clipped[0], clipped[1]); break;
							case 2: nTrisToAdd = test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, clipped[0], clipped[1]); break;
							case 3: nTrisToAdd = test.ClipAgainstPlane({ (float)width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, clipped[0], clipped[1]); break;
						}

						for (int w = 0; w < nTrisToAdd; ++w) {
							clipped[w].z = (clipped[w].p[0].z + clipped[w].p[1].z + clipped[w].p[2].z) / 3.0f;
							listTriangles.push_back(clipped[w]);
						}
					}
					nNewTriangles = listTriangles.size();
				}
				// Add clipped triangles to the final list
				list.insert(list.end(), listTriangles.begin(), listTriangles.end());
			}
		}
	}
	std::lock_guard<std::mutex> lock(*mutex);
	listlistTriangles->insert(listlistTriangles->end(), list.begin(), list.end());
}

void ProcessEngineObject(Engine* engine, EngineObject* obj, std::vector<Triangle>* listlistTriangles, std::mutex* mutex) {
	size_t si = obj->worldmesh.triangles.size() / 2;

	std::vector<Triangle> triangleIn1{ obj->worldmesh.triangles.begin(), obj->worldmesh.triangles.begin() + si };
	std::vector<Triangle> triangleIn2{ obj->worldmesh.triangles.begin() + si, obj->worldmesh.triangles.end() };

	std::thread thread1(ProcessTriangles, engine, &triangleIn1, listlistTriangles, mutex);
	std::thread thread2(ProcessTriangles, engine, &triangleIn2, listlistTriangles, mutex);

	thread1.join();
	thread2.join();
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

	static std::vector<Triangle> listlistTriangles;
	std::unordered_map<std::string, EngineObject>& EngineObjects = world.getEngineObjects();

	static std::mutex mutex;

	std::vector<std::thread*>threads;
	threads.reserve(EngineObjects.size());
	
	for (auto& [name, Object] : EngineObjects) {
		EngineObject* obj = &Object;
		std::thread* thread = new std::thread(ProcessEngineObject, this, obj, &listlistTriangles, (std::mutex*)&mutex);
		threads.push_back(thread);
	}

	for (int i = 0; i < threads.size(); i++) {
		threads[i]->join();
		delete threads[i];
	}

	/* 
		MUST KEEP THIS
	*/
	
	// PUT FRONT TO BACK SORTING ALGO
	// HERE
		
	sort(listlistTriangles.begin(), listlistTriangles.end(), [](Triangle& tri, Triangle& tri2) {
		return tri.z > tri2.z;
	});

	for (const Triangle& tri : listlistTriangles)
	{
		FloodGui::Context.GetBackgroundDrawList()->AddTriangleFilled({ tri.p[0].x, tri.p[0].y }, { tri.p[1].x, tri.p[1].y }, { tri.p[2].x, tri.p[2].y }, tri.col);
	}

	FloodGui::Context.GetForegroundDrawList()->AddText((std::to_string(listlistTriangles.size()) + " triangles").c_str(), { 50, 150 }, FloodColor(255, 0, 0, 255), 20.f, 12.f);

	listlistTriangles.clear();
}