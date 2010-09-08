/*
 * @(#)OGLVertexCache.h	1.2 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLVertexCache_h_Included
#define OGLVertexCache_h_Included

#include "j2d_md.h"
#include "OGLContext.h"

/**
 * Constants that control the size of the vertex cache.
 */
#define OGLVC_MAX_INDEX         1024

/**
 * Constants that control the size of the texture tile cache used for
 * mask operations.
 */
#define OGLVC_MASK_CACHE_TILE_WIDTH       32
#define OGLVC_MASK_CACHE_TILE_HEIGHT      32
#define OGLVC_MASK_CACHE_TILE_SIZE \
   (OGLVC_MASK_CACHE_TILE_WIDTH * OGLVC_MASK_CACHE_TILE_HEIGHT)

#define OGLVC_MASK_CACHE_WIDTH_IN_TILES   8
#define OGLVC_MASK_CACHE_HEIGHT_IN_TILES  4

#define OGLVC_MASK_CACHE_WIDTH_IN_TEXELS \
   (OGLVC_MASK_CACHE_TILE_WIDTH * OGLVC_MASK_CACHE_WIDTH_IN_TILES)
#define OGLVC_MASK_CACHE_HEIGHT_IN_TEXELS \
   (OGLVC_MASK_CACHE_TILE_HEIGHT * OGLVC_MASK_CACHE_HEIGHT_IN_TILES)

/*
 * We reserve one (fully opaque) tile in the upper-right corner for
 * operations where the mask is null.
 */
#define OGLVC_MASK_CACHE_MAX_INDEX \
   ((OGLVC_MASK_CACHE_WIDTH_IN_TILES * OGLVC_MASK_CACHE_HEIGHT_IN_TILES) - 1)
#define OGLVC_MASK_CACHE_SPECIAL_TILE_X \
   (OGLVC_MASK_CACHE_WIDTH_IN_TEXELS - OGLVC_MASK_CACHE_TILE_WIDTH)
#define OGLVC_MASK_CACHE_SPECIAL_TILE_Y \
   (OGLVC_MASK_CACHE_HEIGHT_IN_TEXELS - OGLVC_MASK_CACHE_TILE_HEIGHT)

/**
 * Exported methods.
 */
jboolean OGLVertexCache_InitVertexCache();
void OGLVertexCache_FlushVertexCache();
void OGLVertexCache_RestoreColorState(OGLContext *oglc);

void OGLVertexCache_EnableMaskCache(OGLContext *oglc);
void OGLVertexCache_DisableMaskCache(OGLContext *oglc);
void OGLVertexCache_AddMaskQuad(OGLContext *oglc,
                                jint srcx, jint srcy,
                                jint dstx, jint dsty,
                                jint width, jint height,
                                jint maskscan, void *mask);

void OGLVertexCache_AddGlyphQuad(OGLContext *oglc,
                                 jfloat tx1, jfloat ty1,
                                 jfloat tx2, jfloat ty2,
                                 jfloat dx1, jfloat dy1,
                                 jfloat dx2, jfloat dy2);

#endif /* OGLVertexCache_h_Included */
