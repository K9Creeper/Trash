#pragma once
#include <Windows.h>

#include <d3d11.h>
#include <dxgi1_3.h>

class D3D11WindowInstance {
private:
	struct D3D11 {
		ID3D11Device* device;
		ID3D11DeviceContext* context;
		IDXGISwapChain1* swapChain;
	}_d3d11;
	HWND _hwnd;
public:
	D3D11WindowInstance();
	~D3D11WindowInstance();

	void Create();
	
};