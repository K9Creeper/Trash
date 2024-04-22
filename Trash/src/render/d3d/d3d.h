#pragma once
#include <Windows.h>

#include <d3d9.h>

struct D3DDrawData;
class D3D11WindowInstance {
private:
	struct D3D11 {
		LPDIRECT3D9 d3d;
		LPDIRECT3DDEVICE9 d3ddev;
		D3DPRESENT_PARAMETERS d3dpp;

		D3DDrawData* _drawdata;
	}_d3d11;
	HWND _hwnd;
	int	width = 0, height = 0;
	bool CreateDeviceD3D();
	void CleanupDeviceD3D();
public:
	D3D11WindowInstance(int width, int height);
	~D3D11WindowInstance();
	
	D3D11* getD3DData() { return &_d3d11; }

	void Create();
};