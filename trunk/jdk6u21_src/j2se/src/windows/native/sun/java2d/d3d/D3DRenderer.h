/*
 * @(#)D3DRenderer.h	1.3 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_java2d_pipe_BufferedRenderPipe.h"
#include "D3DContext.h"

#define BYTES_PER_POLY_POINT \
    sun_java2d_pipe_BufferedRenderPipe_BYTES_PER_POLY_POINT
#define BYTES_PER_SCANLINE \
    sun_java2d_pipe_BufferedRenderPipe_BYTES_PER_SCANLINE
#define BYTES_PER_SPAN \
    sun_java2d_pipe_BufferedRenderPipe_BYTES_PER_SPAN

HRESULT D3DPIPELINE_API
D3DRenderer_DrawLine(D3DContext *d3dc,
                     jint x1, jint y1, jint x2, jint y2);

HRESULT D3DPIPELINE_API
D3DRenderer_DrawRect(D3DContext *d3dc,
                     jint x, jint y, jint w, jint h);

HRESULT D3DPIPELINE_API
D3DRenderer_FillRect(D3DContext *d3dc,
                     jint x, jint y, jint w, jint h);

HRESULT D3DPIPELINE_API
D3DRenderer_DrawPoly(D3DContext *d3dc,
                     jint nPoints, jboolean isClosed,
                     jint transX, jint transY,
                     jint *xPoints, jint *yPoints);

HRESULT D3DPIPELINE_API
D3DRenderer_DrawScanlines(D3DContext *d3dc,
                          jint scanlineCount, jint *scanlines);

HRESULT D3DPIPELINE_API
D3DRenderer_FillSpans(D3DContext *d3dc, jint spanCount, jint *spans);

HRESULT D3DPIPELINE_API
D3DRenderer_FillParallelogram(D3DContext *d3dc,
                              jfloat fx11, jfloat fy11,
                              jfloat dx21, jfloat dy21,
                              jfloat dx12, jfloat dy12);

HRESULT D3DPIPELINE_API
D3DRenderer_DrawParallelogram(D3DContext *d3dc,
                              jfloat fx11, jfloat fy11,
                              jfloat dx21, jfloat dy21,
                              jfloat dx12, jfloat dy12,
                              jfloat lw21, jfloat lw12);

HRESULT D3DPIPELINE_API
D3DRenderer_FillAAParallelogram(D3DContext *d3dc,
                                jfloat fx11, jfloat fy11,
                                jfloat dx21, jfloat dy21,
                                jfloat dx12, jfloat dy12);

HRESULT D3DPIPELINE_API
D3DRenderer_DrawAAParallelogram(D3DContext *d3dc,
                                jfloat fx11, jfloat fy11,
                                jfloat dx21, jfloat dy21,
                                jfloat dx12, jfloat dy12,
                                jfloat lw21, jfloat lw12);
