/*
 * @(#)WGLSurfaceData.h	1.2 04/04/14
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef WGLSurfaceData_h_Included
#define WGLSurfaceData_h_Included

#include "OGLSurfaceData.h"
#include "J2D_GL/gl.h"

/**
 * The WGLSDOps structure contains the WGL-specific information for a given
 * OGLSurfaceData.  It is referenced by the native OGLSDOps structure.
 */
typedef struct _WGLSDOps {
    jobject peer;
    union {
        HWND        window;
        HPBUFFERARB pbuffer;
    } drawable;
    HDC pbufferDC;
} WGLSDOps;

#endif /* WGLSurfaceData_h_Included */
