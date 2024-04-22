#include "d3d.h"
#include "../draw/draw.h"

#pragma comment(lib, "d3d9.lib")

D3D11WindowInstance::D3D11WindowInstance(int width, int height)
{
	this->width = width;
	this->height = height;

	_d3d11._drawdata = new D3DDrawData();
	_d3d11._drawdata->iWindow = this;
}

D3D11WindowInstance::~D3D11WindowInstance()
{

}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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


bool D3D11WindowInstance::CreateDeviceD3D() {
	if ((_d3d11.d3d = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
		return false;

	// Create the D3DDevice
	ZeroMemory(&_d3d11.d3dpp, sizeof(_d3d11.d3dpp));
	_d3d11.d3dpp.Windowed = TRUE;
	_d3d11.d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	_d3d11.d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	_d3d11.d3dpp.EnableAutoDepthStencil = TRUE;
	_d3d11.d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	_d3d11.d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (_d3d11.d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &_d3d11.d3dpp, &_d3d11.d3ddev) < 0)
		return false;

	return true;
}
void D3D11WindowInstance::CleanupDeviceD3D() {
	if (_d3d11.d3ddev) { _d3d11.d3ddev->Release(); _d3d11.d3ddev = nullptr; }
	if (_d3d11.d3d) { _d3d11.d3d->Release(); _d3d11.d3d = nullptr; }
}

void D3D11WindowInstance::Create() {
	WNDCLASSEXW wc;
	{
		wc = { sizeof(wc), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TrashClass", nullptr };
		RegisterClassExW(&wc);
		_hwnd = ::CreateWindowW(wc.lpszClassName, L"Trash", WS_OVERLAPPEDWINDOW, 100, 100, width, height, nullptr, nullptr, wc.hInstance, nullptr);

		if (!CreateDeviceD3D())
		{
			CleanupDeviceD3D();
			UnregisterClassW(wc.lpszClassName, wc.hInstance);
			return;
		}

		ShowWindow(_hwnd, SW_SHOWDEFAULT);
		UpdateWindow(_hwnd);

		MSG msg;
		bool running = true;
		while (running) {
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); if (msg.message == WM_QUIT) { CleanupDeviceD3D(); running = false; } }
		}
	}
	CleanupDeviceD3D();
}