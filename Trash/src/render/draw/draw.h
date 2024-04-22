#pragma once
#include <d3d9.h>

#include <cstddef>
#include <iostream>

class D3D11WindowInstance;

struct D3DDrawData {
	D3D11WindowInstance*		iWindow;

	LPDIRECT3DVERTEXBUFFER9     pVB;
	LPDIRECT3DINDEXBUFFER9      pIB;

	int                         VertexBufferSize;
	int                         IndexBufferSize;

	void Init();
	void Shutdown();

	D3DDrawData() {
		memset((void*)this, 0, sizeof(*this));
		VertexBufferSize = 5000;
		IndexBufferSize = 10000;
	}
};