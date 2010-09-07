/*
 * @(#)WGLGraphicsConfig.h	1.1 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef WGLGraphicsConfig_h_Included
#define WGLGraphicsConfig_h_Included

#include "jni.h"
#include "J2D_GL/gl.h"
#include "OGLSurfaceData.h"

/**
 * The WGLGraphicsConfigInfo structure contains information specific to a
 * given WGLGraphicsConfig (pixel format).
 *
 *     OGLExtInfo extInfo;
 * This structure contains flags that indicate whether a particular
 * extension (or logical group of extensions) is available for this
 * WGLGraphicsConfig.
 *
 *     jboolean isDoubleBuffered;
 * If true, indicates that this WGLGraphicsConfig is double-buffered.
 */
typedef struct _WGLGraphicsConfigInfo {
    jint       screen;
    jint       pixfmt;
    OGLExtInfo extInfo;
    jboolean   isDoubleBuffered;
} WGLGraphicsConfigInfo;

/**
 * The WGLCtxInfo structure contains the native WGLContext information
 * required by and is encapsulated by the platform-independent OGLContext
 * structure.
 *
 *     WGLGraphicsConfigInfo configInfo;
 * The native structure that describes the WGLGraphicsConfig under which
 * this context was created.
 *
 *     HGLRC context;
 * The core native WGL context.  Rendering commands have no effect until a
 * context is made current (active).
 */
typedef struct _WGLCtxInfo {
    WGLGraphicsConfigInfo *configInfo;
    HGLRC                  context;
} WGLCtxInfo;

OGLContext *
WGLGC_InitOGLContext(JNIEnv *env, WGLGraphicsConfigInfo *glxinfo,
                     jboolean useDisposer);

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
