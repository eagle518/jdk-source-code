/*
 * @(#)D3DMaskCache.h	1.2 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef D3DMASKCACHE_H
#define D3DMASKCACHE_H

#include "jni.h"
#include "D3DContext.h"

/**
 * Constants that control the size of the texture tile cache used for
 * mask operations.
 */
#define D3D_MASK_CACHE_TILE_WIDTH       32
#define D3D_MASK_CACHE_TILE_HEIGHT      32
#define D3D_MASK_CACHE_TILE_SIZE \
   (D3D_MASK_CACHE_TILE_WIDTH * D3D_MASK_CACHE_TILE_HEIGHT)

#define D3D_MASK_CACHE_WIDTH_IN_TILES   8
#define D3D_MASK_CACHE_HEIGHT_IN_TILES  4

#define D3D_MASK_CACHE_WIDTH_IN_TEXELS \
   (D3D_MASK_CACHE_TILE_WIDTH * D3D_MASK_CACHE_WIDTH_IN_TILES)
#define D3D_MASK_CACHE_HEIGHT_IN_TEXELS \
   (D3D_MASK_CACHE_TILE_HEIGHT * D3D_MASK_CACHE_HEIGHT_IN_TILES)

/*
 * We reserve one (fully opaque) tile in the upper-right corner for
 * operations where the mask is null.
 */
#define D3D_MASK_CACHE_MAX_INDEX \
   ((D3D_MASK_CACHE_WIDTH_IN_TILES * D3D_MASK_CACHE_HEIGHT_IN_TILES) - 1)
#define D3D_MASK_CACHE_SPECIAL_TILE_X \
   (D3D_MASK_CACHE_WIDTH_IN_TEXELS - D3D_MASK_CACHE_TILE_WIDTH)
#define D3D_MASK_CACHE_SPECIAL_TILE_Y \
   (D3D_MASK_CACHE_HEIGHT_IN_TEXELS - D3D_MASK_CACHE_TILE_HEIGHT)

class D3DContext;

class D3DMaskCache {
public:
    HRESULT Init(D3DContext *pCtx);
    void    ReleaseDefPoolResources() {};
            ~D3DMaskCache();
    HRESULT Enable();
    HRESULT Disable();
    HRESULT AddMaskQuad(int srcx, int srcy,
                        int dstx, int dsty,
                        int width, int height,
                        int maskscan, void *mask);

static
    HRESULT CreateInstance(D3DContext *pCtx, D3DMaskCache **ppMaskCache);

private:
               D3DMaskCache();
    UINT       maskCacheIndex;
    D3DContext *pCtx;
};

#endif // D3DMASKCACHE_H
