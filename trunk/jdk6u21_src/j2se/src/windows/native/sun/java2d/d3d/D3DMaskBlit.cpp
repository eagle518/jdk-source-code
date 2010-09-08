/*
 * @(#)D3DMaskBlit.cpp	1.2 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <jlong.h>

#include "D3DMaskBlit.h"
#include "D3DRenderQueue.h"
#include "D3DSurfaceData.h"

/**
 * REMIND: This method assumes that the dimensions of the incoming pixel
 *         array are less than or equal to the cached blit texture tile;
 *         these are rather fragile assumptions, and should be cleaned up...
 */
HRESULT
D3DMaskBlit_MaskBlit(JNIEnv *env, D3DContext *d3dc,
                     jint dstx, jint dsty,
                     jint width, jint height,
                     void *pPixels)
{
    HRESULT res = S_OK;
    jfloat dx1, dy1, dx2, dy2;
    jfloat tx1, ty1, tx2, ty2;

    J2dTraceLn(J2D_TRACE_INFO, "D3DMaskBlit_MaskBlit");

    if (width <= 0 || height <= 0) {
        J2dTraceLn(J2D_TRACE_WARNING,
                   "D3DMaskBlit_MaskBlit: invalid dimensions");
        return res;
    }

    RETURN_STATUS_IF_NULL(pPixels, E_FAIL);
    RETURN_STATUS_IF_NULL(d3dc, E_FAIL);
    if (FAILED(res = d3dc->BeginScene(STATE_TEXTUREOP))) {
        return res;
    }

    D3DResource *pBlitTexRes;
    if (FAILED(res =
               d3dc->GetResourceManager()->GetBlitTexture(&pBlitTexRes)))
    {
        return res;
    }
    IDirect3DTexture9 *pBlitTex = pBlitTexRes->GetTexture();

    if (FAILED(res = d3dc->SetTexture(pBlitTex, 0))) {
        return res;
    }

    IDirect3DDevice9 *pd3dDevice = d3dc->Get3DDevice();
    D3DTEXTUREFILTERTYPE fhint =
        d3dc->IsTextureFilteringSupported(D3DTEXF_NONE) ?
            D3DTEXF_NONE : D3DTEXF_POINT;
    pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, fhint);
    pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, fhint);

    // copy system memory IntArgbPre surface into cached texture
    if (FAILED(res = d3dc->UploadTileToTexture(pBlitTexRes, pPixels,
                                               0, 0, 0, 0,
                                               width, height,
                                               width*4,
                                               TILEFMT_4BYTE_ARGB_PRE)))
    {
        return res;
    }

    dx1 = (jfloat)dstx;
    dy1 = (jfloat)dsty;
    dx2 = dx1 + width;
    dy2 = dy1 + height;

    tx1 = 0.0f;
    ty1 = 0.0f;
    tx2 = ((jfloat)width) / D3DC_BLIT_TILE_SIZE;
    ty2 = ((jfloat)height) / D3DC_BLIT_TILE_SIZE;

    // render cached texture to the destination surface
    res = d3dc->pVCacher->DrawTexture(dx1, dy1, dx2, dy2,
                                      tx1, ty1, tx2, ty2);
    res = d3dc->pVCacher->Render();

    return res;
}
