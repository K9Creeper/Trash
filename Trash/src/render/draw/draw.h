#pragma once
#include <d3d9.h>

#include "../math/vector.h"
#include "../math/color.h"

#include <cstddef>
#include <iostream>
#include <vector>

class D3D11WindowInstance;

struct DrawMaterial {
	std::vector<Vector2>Points{};
	Color col;
	float thinkness;

	int index_count;
	int vertex_count; // equal to points size
};

typedef unsigned short DrawIndex;
struct DrawVertex {
	Vector2 position;
	Vector2 uv;

	unsigned int col = 0x00000000;
};
struct DrawList {
public:
	std::vector< DrawMaterial >Elements{};

	std::vector< DrawVertex > VertexBuffer;
	std::vector< DrawIndex > IndexBuffer;

	DrawIndex* IndexWrite;
	DrawVertex* VertexWrite;

	unsigned int VertexCurrentIdx = 0;

	int GetElementCount() const {
		int ttl = 0;
		for (const DrawMaterial& mat : Elements)
			ttl += mat.Points.size();
		return ttl;
	}

	int GetVertexCount() const {
		return VertexBuffer.size();
	}

	int GetIndexCount() const {
		return IndexBuffer.size();
	}

	void Clear()
	{
		IndexWrite = nullptr;
		VertexWrite = nullptr;
		VertexBuffer.clear();
		IndexBuffer.clear();
		Elements.clear();
		VertexCurrentIdx = 0;
	}
private:
};

struct D3DDrawData {
	D3D11WindowInstance*		iWindow;

	LPDIRECT3DVERTEXBUFFER9     pVB;
	LPDIRECT3DINDEXBUFFER9      pIB;

	int                         VertexBufferSize;
	int                         IndexBufferSize;

	D3DDrawData() {
		memset((void*)this, 0, sizeof(*this));
		VertexBufferSize = 5000;
		IndexBufferSize = 10000;
	}
};

struct DrawData {
	DrawList _drawlist;
	D3DDrawData _d3ddrawdata;

	void Init();
	void Shutdown();

	void NewFrame();
	void RenderData();
};