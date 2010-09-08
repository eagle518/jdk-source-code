/*
 * @(#)D3DSurfaceData.h	1.8 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _D3DSURFACEDATA_H_
#define _D3DSURFACEDATA_H_

#include "java_awt_image_AffineTransformOp.h"
#include "sun_java2d_d3d_D3DSurfaceData.h"
#include "sun_java2d_pipe_hw_AccelSurface.h"
#include "SurfaceData.h"
#include <d3d9.h>

typedef struct _D3DSDOps D3DSDOps;

class D3DResource;

struct _D3DSDOps {
    SurfaceDataOps sdOps;

    // the ordinal of the d3d adapter this surface belongs to
    // (may be different from GDI display number)
    jint adapter;
    jint width, height;

    // backbuffer-related data
    jint xoff, yoff;
    D3DSWAPEFFECT swapEffect;

    D3DResource  *pResource;
};

#define UNDEFINED       sun_java2d_pipe_hw_AccelSurface_UNDEFINED
#define RT_PLAIN        sun_java2d_pipe_hw_AccelSurface_RT_PLAIN
#define TEXTURE         sun_java2d_pipe_hw_AccelSurface_TEXTURE
#define RT_TEXTURE      sun_java2d_pipe_hw_AccelSurface_RT_TEXTURE
#define FLIP_BACKBUFFER sun_java2d_pipe_hw_AccelSurface_FLIP_BACKBUFFER
#define D3D_DEVICE_RESOURCE \
                        sun_java2d_d3d_D3DSurfaceData_D3D_DEVICE_RESOURCE

#define ST_INT_ARGB        sun_java2d_d3d_D3DSurfaceData_ST_INT_ARGB
#define ST_INT_ARGB_PRE    sun_java2d_d3d_D3DSurfaceData_ST_INT_ARGB_PRE
#define ST_INT_ARGB_BM     sun_java2d_d3d_D3DSurfaceData_ST_INT_ARGB_BM
#define ST_INT_RGB         sun_java2d_d3d_D3DSurfaceData_ST_INT_RGB
#define ST_INT_BGR         sun_java2d_d3d_D3DSurfaceData_ST_INT_BGR
#define ST_USHORT_565_RGB  sun_java2d_d3d_D3DSurfaceData_ST_USHORT_565_RGB
#define ST_USHORT_555_RGB  sun_java2d_d3d_D3DSurfaceData_ST_USHORT_555_RGB
#define ST_BYTE_INDEXED    sun_java2d_d3d_D3DSurfaceData_ST_BYTE_INDEXED
#define ST_BYTE_INDEXED_BM sun_java2d_d3d_D3DSurfaceData_ST_BYTE_INDEXED_BM
#define ST_3BYTE_BGR       sun_java2d_d3d_D3DSurfaceData_ST_3BYTE_BGR

/**
 * These are defined to be the same as ExtendedBufferCapabilities.VSyncType
 * enum.
 */
#define VSYNC_DEFAULT 0
#define VSYNC_ON      1
#define VSYNC_OFF     2

/**
 * These are shorthand names for the filtering method constants used by
 * image transform methods.
 */
#define D3DSD_XFORM_DEFAULT 0
#define D3DSD_XFORM_NEAREST_NEIGHBOR \
    java_awt_image_AffineTransformOp_TYPE_NEAREST_NEIGHBOR
#define D3DSD_XFORM_BILINEAR \
    java_awt_image_AffineTransformOp_TYPE_BILINEAR

void D3DSD_Flush(void *pData);
void D3DSD_MarkLost(void *pData);

#endif /* _D3DSURFACEDATA_H_ */
