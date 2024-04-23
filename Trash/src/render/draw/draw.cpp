#include "draw.h"

#include "../d3d/d3d.h"

struct CUSTOMVERTEX
{
    float    pos[3];
    D3DCOLOR col;
    float    uv[2];
};

void DrawData::Init()
{
    _d3ddrawdata.iWindow->getD3DData()->d3ddev->AddRef();
}

void DrawData::Shutdown()
{
    if (_d3ddrawdata.iWindow->getD3DData()->d3ddev)
        return;
    if (_d3ddrawdata.pVB) { _d3ddrawdata.pVB->Release(); _d3ddrawdata.pVB = nullptr; }
    if (_d3ddrawdata.pIB) { _d3ddrawdata.pIB->Release(); _d3ddrawdata.pIB = nullptr; }
    if (_d3ddrawdata.iWindow->getD3DData()->d3ddev) { _d3ddrawdata.iWindow->getD3DData()->d3ddev->Release(); }
}

void DrawData::NewFrame() {
    _drawlist.Clear();
}

void DrawData::RenderData() {

    // We extend the buffer size if needed
    if (!_d3ddrawdata.pVB || _d3ddrawdata.VertexBufferSize < _drawlist.GetVertexCount())
    {
        if (_d3ddrawdata.pVB) { _d3ddrawdata.pVB->Release(); _d3ddrawdata.pVB = nullptr; }
        _d3ddrawdata.VertexBufferSize = _drawlist.GetVertexCount() + 5000;
        if (_d3ddrawdata.iWindow->getD3DData()->d3ddev->CreateVertexBuffer(_d3ddrawdata.VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1), D3DPOOL_DEFAULT, &_d3ddrawdata.pVB, nullptr) < 0)
            return;
    }
    if (!_d3ddrawdata.pIB || _d3ddrawdata.IndexBufferSize < _drawlist.GetIndexCount())
    {
        if (_d3ddrawdata.pIB) { _d3ddrawdata.pIB->Release(); _d3ddrawdata.pIB = nullptr; }
        _d3ddrawdata.IndexBufferSize = _drawlist.GetIndexCount() + 10000;
        if (_d3ddrawdata.iWindow->getD3DData()->d3ddev->CreateIndexBuffer(_d3ddrawdata.IndexBufferSize * sizeof(DrawIndex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(DrawIndex) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &_d3ddrawdata.pIB, nullptr) < 0)
            return;
    }

    // We should store the d3d state
    IDirect3DStateBlock9* d3d9_state_block = nullptr;
    if (_d3ddrawdata.iWindow->getD3DData()->d3ddev->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
        return;
    if (d3d9_state_block->Capture() < 0)
    {
        d3d9_state_block->Release();
        return;
    }

    D3DMATRIX last_world, last_view, last_projection;
    _d3ddrawdata.iWindow->getD3DData()->d3ddev->GetTransform(D3DTS_WORLD, &last_world);
    _d3ddrawdata.iWindow->getD3DData()->d3ddev->GetTransform(D3DTS_VIEW, &last_view);
    _d3ddrawdata.iWindow->getD3DData()->d3ddev->GetTransform(D3DTS_PROJECTION, &last_projection);

    CUSTOMVERTEX* vtx_dst;
    DrawIndex* idx_dst;
    if (_d3ddrawdata.pVB->Lock(0, (UINT)(_drawlist.GetVertexCount() * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
    {
        d3d9_state_block->Release();
        return;
    }
    if (_d3ddrawdata.pIB->Lock(0, (UINT)(_drawlist.GetIndexCount() * sizeof(DrawIndex)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
    {
        _d3ddrawdata.pVB->Unlock();
        d3d9_state_block->Release();
        return;
    }
    // We are filling up the global drawlist
    //
    {
        const DrawList* cmd_list = &_drawlist;
        const DrawVertex* vtx_src = cmd_list->VertexBuffer.data();
        for (int i = 0; i < cmd_list->VertexBuffer.size(); i++)
        {
            vtx_dst->pos[0] = vtx_src->position.x;
            vtx_dst->pos[1] = vtx_src->position.y;
            vtx_dst->pos[2] = 0.0f;
            vtx_dst->col = vtx_src->col;
            vtx_dst->uv[0] = vtx_src->uv.x;
            vtx_dst->uv[1] = vtx_src->uv.y;
            vtx_dst++;
            vtx_src++;
        }
        memcpy(idx_dst, cmd_list->IndexBuffer.data(), cmd_list->IndexBuffer.size() * sizeof(DrawIndex));
        idx_dst += cmd_list->IndexBuffer.size();
    }

    _d3ddrawdata.pVB->Unlock();
    _d3ddrawdata.pIB->Unlock();
    _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetStreamSource(0, _d3ddrawdata.pVB, 0, sizeof(CUSTOMVERTEX));
    _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetIndices(_d3ddrawdata.pIB);
    _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetFVF((D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1));

    // Here we are setting up the veiwport and rendering
    {
        D3DVIEWPORT9 vp;
        vp.X = vp.Y = 0;
        vp.Width = (DWORD)((int)_d3ddrawdata.iWindow->getDisplay()->size.x);
        vp.Height = (DWORD)((int)_d3ddrawdata.iWindow->getDisplay()->size.y);
        vp.MinZ = 0.0f;
        vp.MaxZ = 1.0f;
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetViewport(&vp);

        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetPixelShader(nullptr);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetVertexShader(nullptr);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_FOGENABLE, FALSE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_CLIPPING, TRUE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

        // We set up our viewmatrix..this is very important
        {
            D3DMATRIX mat_identity = { { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } } };
            D3DMATRIX mat_projection = _d3ddrawdata.iWindow->getDisplay()->matrix_project();

            _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTransform(D3DTS_WORLD, &mat_identity);
            _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTransform(D3DTS_VIEW, &mat_identity);
            _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTransform(D3DTS_PROJECTION, &mat_projection);
        }
    }

    {
        const DrawList* cmd_list = &_drawlist;

        int vtx_offset = 0;
        int idx_offset = 0;

        // We are looping through practically every cmd to the gpu
        for (int i = 0; i < cmd_list->Elements.size(); i++) {
            const DrawMaterial& material = cmd_list->Elements[i];
            
            _d3ddrawdata.iWindow->getD3DData()->d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list->VertexBuffer.size(), idx_offset, material.index_count / 3);
            
            idx_offset += material.index_count;
        }
    }

    // Now can begin resoring
    _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTransform(D3DTS_WORLD, &last_world);
    _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTransform(D3DTS_VIEW, &last_view);
    _d3ddrawdata.iWindow->getD3DData()->d3ddev->SetTransform(D3DTS_PROJECTION, &last_projection);

    // Restore the DX9 state
    d3d9_state_block->Apply();
    d3d9_state_block->Release();

    //FloodGui::Context.FrameData.tFrameEnd = std::chrono::system_clock::now().time_since_epoch().count();
}