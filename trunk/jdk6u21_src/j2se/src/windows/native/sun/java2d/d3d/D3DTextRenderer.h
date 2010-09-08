/*
 * @(#)D3DTextRenderer.h	1.2 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef D3DTextRenderer_h_Included
#define D3DTextRenderer_h_Included

#include <jni.h>
#include <jlong.h>
#include "sun_java2d_pipe_BufferedTextPipe.h"
#include "AccelGlyphCache.h"
#include "D3DContext.h"
#include "D3DSurfaceData.h"

/**
 * The following constants define the inner and outer bounds of the
 * accelerated glyph cache.
 */
#define D3DTR_CACHE_WIDTH       512
#define D3DTR_CACHE_HEIGHT      512
#define D3DTR_CACHE_CELL_WIDTH  16
#define D3DTR_CACHE_CELL_HEIGHT 16

/**
 * This constant defines the size of the tile to use in the
 * D3DTR_DrawLCDGlyphNoCache() method.  See below for more on why we
 * restrict this value to a particular size.
 */
#define D3DTR_NOCACHE_TILE_SIZE 32

/**
 * These constants define the size of the "cached destination" texture.
 * This texture is only used when rendering LCD-optimized text, as that
 * codepath needs direct access to the destination.  There is no way to
 * access the framebuffer directly from a Direct3D shader, so we need to first
 * copy the destination region corresponding to a particular glyph into
 * this cached texture, and then that texture will be accessed inside the
 * shader.  Copying the destination into this cached texture can be a very
 * expensive operation (accounting for about half the rendering time for
 * LCD text), so to mitigate this cost we try to bulk read a horizontal
 * region of the destination at a time.  (These values are empirically
 * derived for the common case where text runs horizontally.)
 *
 * Note: It is assumed in various calculations below that:
 *     (D3DTR_CACHED_DEST_WIDTH  >= D3DTR_CACHE_CELL_WIDTH)  &&
 *     (D3DTR_CACHED_DEST_WIDTH  >= D3DTR_NOCACHE_TILE_SIZE) &&
 *     (D3DTR_CACHED_DEST_HEIGHT >= D3DTR_CACHE_CELL_HEIGHT) &&
 *     (D3DTR_CACHED_DEST_HEIGHT >= D3DTR_NOCACHE_TILE_SIZE)
 */
#define D3DTR_CACHED_DEST_WIDTH  512
#define D3DTR_CACHED_DEST_HEIGHT 32

#define BYTES_PER_GLYPH_IMAGE \
    sun_java2d_pipe_BufferedTextPipe_BYTES_PER_GLYPH_IMAGE
#define BYTES_PER_GLYPH_POSITION \
    sun_java2d_pipe_BufferedTextPipe_BYTES_PER_GLYPH_POSITION
#define BYTES_PER_POSITIONED_GLYPH \
    (BYTES_PER_GLYPH_IMAGE + BYTES_PER_GLYPH_POSITION)

#define OFFSET_CONTRAST  sun_java2d_pipe_BufferedTextPipe_OFFSET_CONTRAST
#define OFFSET_RGBORDER  sun_java2d_pipe_BufferedTextPipe_OFFSET_RGBORDER
#define OFFSET_SUBPIXPOS sun_java2d_pipe_BufferedTextPipe_OFFSET_SUBPIXPOS
#define OFFSET_POSITIONS sun_java2d_pipe_BufferedTextPipe_OFFSET_POSITIONS

HRESULT D3DTR_EnableGlyphVertexCache(D3DContext *d3dc);
HRESULT D3DTR_DisableGlyphVertexCache(D3DContext *d3dc);

HRESULT D3DTR_DrawGlyphList(D3DContext *d3dc, D3DSDOps *dstOps,
                            jint totalGlyphs, jboolean usePositions,
                            jboolean subPixPos, jboolean rgbOrder,
                            jint lcdContrast,
                            jfloat glyphListOrigX, jfloat glyphListOrigY,
                            unsigned char *images, unsigned char *positions);

#endif /* D3DTextRenderer_h_Included */
