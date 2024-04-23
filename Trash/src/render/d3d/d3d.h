#pragma once
#include <Windows.h>

#include <d3d9.h>
#include "../math/vector.h"

struct DrawData;
class D3D11WindowInstance {
private:
	struct D3D11 {
		LPDIRECT3D9 d3d;
		LPDIRECT3DDEVICE9 d3ddev;
		D3DPRESENT_PARAMETERS d3dpp;

		DrawData* _drawdata;
	}_d3d11;
	HWND _hwnd;
	class Display {
	public:
		Display() { }
		Vector2	size;
		Vector2 position;
		constexpr D3DMATRIX matrix_project() const {
			// Orthographic projection matrix
			// I HAVE NO IDEA WHAT IM DOING!!!!
			const float L = position.x + 0.5f;
			const float R = position.x + size.x + 0.5f;
			const float T = position.y + 0.5f;
			const float B = position.y + size.y + 0.5f;

			return { { {
				2.0f / (R - L),   0.0f,         0.0f,  0.0f,
				0.0f,         2.0f / (T - B),   0.0f,  0.0f,
				0.0f,         0.0f,         0.5f,  0.0f,
				(L + R) / (L - R),  (T + B) / (B - T),  0.5f,  1.0f
			} } };
		}
	}_display;
	bool CreateDeviceD3D();
	void CleanupDeviceD3D();
public:
	D3D11WindowInstance(const float& width, const float& height);
	~D3D11WindowInstance();
	
	constexpr D3D11* getD3DData() { return &_d3d11; }
	constexpr Display* getDisplay() { return &_display; }
	void Create();
};