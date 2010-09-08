/*
 * @(#)D3DBlitLoops.h	1.2 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef D3DBlitLoops_h_Included
#define D3DBlitLoops_h_Included

#include "sun_java2d_d3d_D3DBlitLoops.h"
#include "D3DSurfaceData.h"
#include "D3DContext.h"

#define OFFSET_SRCTYPE sun_java2d_d3d_D3DBlitLoops_OFFSET_SRCTYPE
#define OFFSET_HINT    sun_java2d_d3d_D3DBlitLoops_OFFSET_HINT
#define OFFSET_TEXTURE sun_java2d_d3d_D3DBlitLoops_OFFSET_TEXTURE
#define OFFSET_RTT     sun_java2d_d3d_D3DBlitLoops_OFFSET_RTT
#define OFFSET_XFORM   sun_java2d_d3d_D3DBlitLoops_OFFSET_XFORM
#define OFFSET_ISOBLIT sun_java2d_d3d_D3DBlitLoops_OFFSET_ISOBLIT

D3DPIPELINE_API HRESULT
D3DBlitLoops_IsoBlit(JNIEnv *env,
                     D3DContext *d3dc, jlong pSrcOps, jlong pDstOps,
                     jboolean xform, jint hint,
                     jboolean texture, jboolean rtt,
                     jint sx1, jint sy1,
                     jint sx2, jint sy2,
                     jdouble dx1, jdouble dy1,
                     jdouble dx2, jdouble dy2);

D3DPIPELINE_API HRESULT
D3DBL_CopySurfaceToIntArgbImage(IDirect3DSurface9 *pSurface,
                                SurfaceDataRasInfo *pDstInfo,
                                jint srcx, jint srcy,
                                jint srcWidth, jint srcHeight,
                                jint dstx, jint dsty);

D3DPIPELINE_API HRESULT
D3DBL_CopyImageToIntXrgbSurface(SurfaceDataRasInfo *pSrcInfo,
                                int srctype,
                                D3DResource *pDstSurfaceRes,
                                jint srcx, jint srcy,
                                jint srcWidth, jint srcHeight,
                                jint dstx, jint dsty);

HRESULT
D3DBlitLoops_Blit(JNIEnv *env,
                  D3DContext *d3dc, jlong pSrcOps, jlong pDstOps,
                  jboolean xform, jint hint,
                  jint srctype, jboolean texture,
                  jint sx1, jint sy1,
                  jint sx2, jint sy2,
                  jdouble dx1, jdouble dy1,
                  jdouble dx2, jdouble dy2);

HRESULT
D3DBlitLoops_SurfaceToSwBlit(JNIEnv *env, D3DContext *d3dc,
                             jlong pSrcOps, jlong pDstOps, jint dsttype,
                             jint srcx, jint srcy,
                             jint dstx, jint dsty,
                             jint width, jint height);

HRESULT
D3DBlitLoops_CopyArea(JNIEnv *env,
                      D3DContext *d3dc, D3DSDOps *dstOps,
                      jint x, jint y,
                      jint width, jint height,
                      jint dx, jint dy);

#endif /* D3DBlitLoops_h_Included */
