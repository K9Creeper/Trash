#include "render.h"
#include "floodgui/flood_gui.h"
#include "floodgui/flood_gui_win.h"

#include "math/mesh.h"

FloodColor clearColor(1.f, 1.f, 1.f, 1.f);

extern LRESULT CALLBACK FloodGuiWindowWinProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (FloodGuiWindowWinProcHandler(hwnd, message, wParam, lParam))
		return TRUE;

	switch (message)
	{
	case WM_PAINT:
	{

		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
		break;
	}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void Window::CreateFlood() {
	if (_hwnd)
		return;
	{
		wc = { sizeof(wc), CS_CLASSDC, MainWindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TrashClass", nullptr };
		RegisterClassExW(&wc);
		_hwnd = ::CreateWindowW(wc.lpszClassName, L"Trash", WS_OVERLAPPEDWINDOW, 0, 0, 800, 700, nullptr, nullptr, wc.hInstance, nullptr);

		// Initialize Direct3D
		if (!CreateDeviceD3D())
		{
			CleanupDeviceD3D();
			UnregisterClassW(wc.lpszClassName, wc.hInstance);
			return;
		}

		// Show the window
		ShowWindow(_hwnd, SW_SHOWDEFAULT);
		UpdateWindow(_hwnd);
	}
	FloodGui::SetupColorStyle();
	FloodGuiWinInit(_hwnd);
	FloodGuiD3D9Init(d3ddev);

	RECT hwndRect = { 0, 0, 0, 0 };
	GetClientRect(_hwnd, &hwndRect);
	FloodGui::Context.Display.DisplaySize = FloodVector2{ static_cast<float>(hwndRect.right - hwndRect.left), static_cast<float>(hwndRect.bottom - hwndRect.top) };
	FloodGui::Context.Display.DisplayPosition = FloodVector2{ static_cast<float>(hwndRect.left), static_cast<float>(hwndRect.top) };

	FloodGui::Context.DrawData = new FloodDrawData(&FloodGui::Context.Display);
}
void Window::RunAndAttachFlood(std::function<void()> handle) {
	MSG msg;

	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); if (msg.message == WM_QUIT) { CleanupDeviceD3D(); running = false; } }
		if (!running)
			break;

		FloodGuiWinNewFrame();
		FloodGuiD3D9NewFrame();

		FloodGui::NewFrame();
		{
			handle();
		}
		FloodGui::EndFrame();
		d3ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);


		d3ddev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor.ToU32(), 1.0f, 0);

		if (d3ddev->BeginScene() >= 0)
		{
			FloodGui::Render();
			FloodGuiD3D9RenderDrawData(FloodGui::Context.DrawData);
			d3ddev->EndScene();
		}

		HRESULT result = d3ddev->Present(nullptr, nullptr, nullptr, nullptr);
	}

	FloodGuiD3D9Shutdown();
	FloodGuiWinShutdown();

	DestroyWindow(_hwnd);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

bool Window::CreateDeviceD3D()
{
	if ((d3d = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
		return false;

	// Create the D3DDevice
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3ddev) < 0)
		return false;

	return true;
}

void Window::CleanupDeviceD3D()
{
	if (d3ddev) { d3ddev->Release(); d3ddev = nullptr; }
	if (d3d) { d3d->Release(); d3d = nullptr; }
}

Render::Render() { window = new Window(); }

void Render::Init() {
	window->CreateFlood();
}

void Render::Begin(std::function<void()> handle) {
	window->RunAndAttachFlood(handle);
}

void Render::DrawTri(float x1, float y1, float x2, float y2, float x3, float y3) {
	FloodGui::Context.GetForegroundDrawList()->AddPolyLine({ {x1, y1}, {x2, y2}, {x3, y3} }, FloodColor());
}