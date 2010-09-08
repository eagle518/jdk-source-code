/*
 * @(#)WGLGraphicsConfig.h	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef WGLGraphicsConfig_h_Included
#define WGLGraphicsConfig_h_Included

#include "jni.h"
#include "J2D_GL/gl.h"
#include "OGLSurfaceData.h"
#include "OGLContext.h"

/**
 * The WGLGraphicsConfigInfo structure contains information specific to a
 * given WGLGraphicsConfig (pixel format).
 *
 *     jint screen, pixfmt;
 * The screen and PixelFormat for the associated WGLGraphicsConfig.
 *
 *     OGLContext *context;
 * The context associated with this WGLGraphicsConfig.
 */
typedef struct _WGLGraphicsConfigInfo {
    jint       screen;
    jint       pixfmt;
    OGLContext *context;
} WGLGraphicsConfigInfo;

/**
 * The WGLCtxInfo structure contains the native WGLContext information
 * required by and is encapsulated by the platform-independent OGLContext
 * structure.
 *
 *     HGLRC context;
 * The core native WGL context.  Rendering commands have no effect until a
 * context is made current (active).
 *
 *     HPBUFFERARB scratchSurface;
 *     HDC         scratchSurfaceDC;
 * The scratch surface (and its associated HDC), which are used to make a
 * context current when we do not otherwise have a reference to an OpenGL
 * surface for the purposes of making a context current.
 */
typedef struct _WGLCtxInfo {
    HGLRC       context;
    HPBUFFERARB scratchSurface;
    HDC         scratchSurfaceDC;
} WGLCtxInfo;

/**
 * Utility methods
 */
HWND WGLGC_CreateScratchWindow(jint screennum);

/**
 * REMIND: ideally, this would be declared in AwtComponent.h, but including
 *         that C++ header file from C source files causes problems...
 */
extern HWND AwtComponent_GetHWnd(JNIEnv *env, jlong pData);

#endif /* WGLGraphicsConfig_h_Included */
