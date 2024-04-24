#pragma once
#include <Windows.h>
#include <d3d9.h>

#include <functional>

class FloodVector2;
class FloodColor;

class Window {
private:
	LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
	LPDIRECT3DDEVICE9 d3ddev;
	D3DPRESENT_PARAMETERS d3dpp;

	HWND _hwnd;
	WNDCLASSEXW wc;

	bool CreateDeviceD3D();
	void CleanupDeviceD3D();

	bool running = true;
public:
	void CreateFlood();
	void RunAndAttachFlood(std::function<void()> handle);
	void Destroy() { running = false; }
	LPDIRECT3DDEVICE9 getD3DDev() { return d3ddev; }
};

class Render {
private:
	Window* window;
public:
	Render();
	void Init();
	void Begin(std::function<void()> handle);

	void DrawTri(float x1, float y1, float x2, float y2, float x3, float y3);

	Window* getWindow() { return window; }
};