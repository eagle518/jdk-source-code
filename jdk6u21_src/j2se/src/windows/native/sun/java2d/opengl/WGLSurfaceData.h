/*
 * @(#)WGLSurfaceData.h	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef WGLSurfaceData_h_Included
#define WGLSurfaceData_h_Included

#include "OGLSurfaceData.h"
#include "WGLGraphicsConfig.h"
#include "J2D_GL/gl.h"

/**
 * The WGLSDOps structure contains the WGL-specific information for a given
 * OGLSurfaceData.  It is referenced by the native OGLSDOps structure.
 */
typedef struct _WGLSDOps {
    WGLGraphicsConfigInfo *configInfo;
    HWND        window;
    HPBUFFERARB pbuffer;
    HDC pbufferDC;
} WGLSDOps;

#endif /* WGLSurfaceData_h_Included */
