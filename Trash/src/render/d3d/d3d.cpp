#include "d3d.h"
#include "../draw/draw.h"

#pragma comment(lib, "d3d9.lib")

D3D11WindowInstance::D3D11WindowInstance(const float& width, const float& height)
{
	this->_display.size = { width, height };
	this->_display.position = { 100, 100};
	_d3d11._drawdata = new DrawData();
	_d3d11._drawdata->_d3ddrawdata.iWindow = this;
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
	static Color clearColor(1.f, 1.f, 1.f, 1.f);
	WNDCLASSEXW wc;
	{
		wc = { sizeof(wc), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TrashClass", nullptr };
		RegisterClassExW(&wc);
		_hwnd = ::CreateWindowW(wc.lpszClassName, L"Trash", WS_OVERLAPPEDWINDOW, _display.position.x, _display.position.y, _display.size.x, _display.size.y, nullptr, nullptr, wc.hInstance, nullptr);

		if (!CreateDeviceD3D())
		{
			CleanupDeviceD3D();
			UnregisterClassW(wc.lpszClassName, wc.hInstance);
			return;
		}

		ShowWindow(_hwnd, SW_SHOWDEFAULT);
		UpdateWindow(_hwnd);

		_d3d11._drawdata->Init();

		MSG msg;
		bool running = true;
		while (running) {
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); if (msg.message == WM_QUIT) { CleanupDeviceD3D(); running = false; } }
			
			if (!running)
				break;
			
			_d3d11._drawdata->NewFrame();

			_d3d11.d3ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
			_d3d11.d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			_d3d11.d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
			
			_d3d11.d3ddev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor.ToU32(), 1.0f, 0);

			if (_d3d11.d3ddev->BeginScene() >= 0)
			{
				_d3d11._drawdata->RenderData();
			}

			HRESULT result = _d3d11.d3ddev->Present(nullptr, nullptr, nullptr, nullptr);
		}

		_d3d11._drawdata->Shutdown();
	}
	DestroyWindow(_hwnd);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
}