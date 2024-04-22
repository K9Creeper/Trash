#include "draw.h"

#include "../d3d/d3d.h"

void D3DDrawData::Init()
{
	iWindow->getD3DData()->d3ddev->AddRef();
}

void D3DDrawData::Shutdown()
{
    if (iWindow->getD3DData()->d3ddev)
        return;
    if (pVB) { pVB->Release(); pVB = nullptr; }
    if (pIB) { pIB->Release(); pIB = nullptr; }
    if (iWindow->getD3DData()->d3ddev) { iWindow->getD3DData()->d3ddev->Release(); }
}