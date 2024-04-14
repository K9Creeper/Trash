#include "d3d.h"

D3D11WindowInstance::D3D11WindowInstance()
{

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

void D3D11WindowInstance::Create() {
    WNDCLASSEXW wc;
    {
        wc.cbSize = sizeof(wc),
            wc.lpfnWndProc = WindowProc;
            wc.hInstance = NULL;
            wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.lpszClassName = L"TrashClass";
    }

}