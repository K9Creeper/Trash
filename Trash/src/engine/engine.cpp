#include "engine.h"
#include "render/render.h"
#include "render/floodgui/flood_gui.h"

#include "trace/trace.h"

#include <thread>
#include <mutex>


Engine::Engine() {
	render = new Render();
}

void InputThread(Engine* engine)
{
	while (engine->running)
	{
		Vector3 vForward = (engine->camera.lookDir * 8.0f * 0.05f);
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

		if (FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse])
		{
			engine->camera.rotation.yaw += .05f;
		}
		if (FloodGui::Context.IO.MouseInput[FloodGuiButton_RightMouse])
		{
			engine->camera.rotation.yaw -= .05f;
		}
		Sleep(16);
	}
}

void Engine::Start() {
	Mesh cube;
	if (cube.LoadFromObjectFile("mountains.obj")) {
		world.AddEngineObject("ground", EngineObject(cube, { 0.f, 0.f, 0.f }));
	}
	
	world.lightSources.push_back({ {0, 15, 0}, 999.9f});

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
		}

		// Calculate triangle normal
		Vector3 line1 = triTransformed.p[1] - triTransformed.p[0];
		Vector3 line2 = triTransformed.p[2] - triTransformed.p[0];
		Vector3 normal = line1.CrossProduct(line2).Normalise();

		float n = normal.DotProduct(triTransformed.p[0] - engine->camera.origin);

		// Check backface culling
		if (n > 0.0f)
			continue;

		// Transform triangle to camera view
		for (int i = 0; i < 3; ++i) {
			triViewed.p[i] = engine->camera.matView.MultiplyVector(triTransformed.p[i]);
			triViewed.t[i] = triTransformed.t[i];
		}

		// Calculate shading / shadowsish
		for (LightSource& ls : engine->world.lightSources) {
			static int n = 0;
			FloodColor col = triViewed.col;

			col = FloodColor(50, 255, 255, 255);
			//shadeColor(col, max(0.1f, direction.Normalise().DotProduct(normal)));
			//if (col.r() > triViewed.col.r() || n == 0) {
			Trace<Triangle> tr;
			tr.collided = false;
			tr.origin = engine->camera.origin;
			tr.direction = engine->camera.lookDir;
			//tr.direction.Normalise();
			tr.TraceLine(engine, allTris);
			
			if (tr.collided && tri == tr.hit)
				triViewed.col = FloodColor(255, 0, 0, 255);
			else
				triViewed.col = col;
			//n++;
		//}
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
			for (int i = 0; i < 3; ++i) {
				clip.finish.t[i] = clip.clipped[n].t[i];
				clip.finish.t[i].x /= clip.finish.p[i].z;
				clip.finish.t[i].y /= clip.finish.p[i].z;
				clip.finish.t[i].z = 1.0f / clip.finish.p[i].z;
				clip.finish.p[i].x *= -1.0f;
				clip.finish.p[i].y *= -1.0f;
				clip.finish.p[i] = clip.finish.p[i] + Vector3{ 1.0f, 1.0f, 0.0f };
				clip.finish.p[i].x *= 0.5f * (float)width;
				clip.finish.p[i].y *= 0.5f * (float)height;
			}

			// Check if triangle is on screen
			clip.valid = pointOnScreen(clip.finish.p[0].x, clip.finish.p[0].y) ||
				pointOnScreen(clip.finish.p[1].x, clip.finish.p[1].y) ||
				pointOnScreen(clip.finish.p[2].x, clip.finish.p[2].y);

			if (clip.valid) {
				// Clip against screen edges
				Triangle clipped[2];
				std::list<Triangle> listTriangles;

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

						for (int w = 0; w < nTrisToAdd; ++w)
							listTriangles.push_back(clipped[w]);
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

	std::mutex mutex;

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

	sort(listlistTriangles.begin(), listlistTriangles.end(), [](Triangle& t1, Triangle& t2) { return (t1.z) > (t2.z); });
		
	for (const Triangle& tri : listlistTriangles)
	{
		
		FloodGui::Context.GetBackgroundDrawList()->AddTriangleFilled({ tri.p[0].x, tri.p[0].y }, { tri.p[1].x, tri.p[1].y }, { tri.p[2].x, tri.p[2].y }, tri.col);
		render->DrawTri( tri.p[0].x, tri.p[0].y, tri.p[1].x, tri.p[1].y, tri.p[2].x, tri.p[2].y);
	}

	FloodGui::Context.GetForegroundDrawList()->AddText((std::to_string(listlistTriangles.size()) + " triangles").c_str(), { 50, 150 }, FloodColor(255, 0, 0, 255), 20.f, 12.f);

	listlistTriangles.clear();
}