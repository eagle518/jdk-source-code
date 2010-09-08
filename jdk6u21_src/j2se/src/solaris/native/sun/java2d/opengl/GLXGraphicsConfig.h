/*
 * @(#)GLXGraphicsConfig.h	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef GLXGraphicsConfig_h_Included
#define GLXGraphicsConfig_h_Included

#include "jni.h"
#include "J2D_GL/glx.h"
#include "OGLSurfaceData.h"
#include "OGLContext.h"

#ifdef HEADLESS
#define GLXGraphicsConfigInfo void
#define GLXCtxInfo void
#else /* HEADLESS */

/**
 * The GLXGraphicsConfigInfo structure contains information specific to a
 * given GLXGraphicsConfig (visual).  Each AwtGraphicsConfigData struct
 * associated with a GLXGraphicsConfig contains a pointer to a
 * GLXGraphicsConfigInfo struct (if it is actually an X11GraphicsConfig, that
 * pointer value will be NULL).
 *
 *     jint screen, visual;
 * The X11 screen and visual IDs for the associated GLXGraphicsConfig.
 *
 *     OGLContext *context;
 * The context associated with this GLXGraphicsConfig.
 *
 *     GLXFBConfig fbconfig;
 * A handle used in many GLX methods for querying certain attributes of the
 * GraphicsConfig (visual), creating new GLXContexts, and creating
 * GLXDrawable surfaces (pbuffers, etc).  Each GraphicsConfig has one
 * associated GLXFBConfig.
 */
typedef struct _GLXGraphicsConfigInfo {
    jint          screen;
    jint          visual;
    OGLContext    *context;
    GLXFBConfig   fbconfig;
} GLXGraphicsConfigInfo;

/**
 * The GLXCtxInfo structure contains the native GLXContext information
 * required by and is encapsulated by the platform-independent OGLContext
 * structure.
 *
 *     GLXContext context;
 * The core native GLX context.  Rendering commands have no effect until a
 * GLXContext is made current (active).
 *
 *     GLXFBConfig fbconfig;
 * This is the same GLXFBConfig that is stored in the GLXGraphicsConfigInfo
 * whence this GLXContext was created.  It is provided here for convenience.
 *
 *     GLXPbuffer  scratchSurface;
 * The scratch surface, which is used to make a context current when we do
 * not otherwise have a reference to an OpenGL surface for the purposes of
 * making a context current.
 */
typedef struct _GLXCtxInfo {
    GLXContext  context;
    GLXFBConfig fbconfig;
    GLXPbuffer  scratchSurface;
} GLXCtxInfo;

jboolean GLXGC_IsGLXAvailable();
VisualID GLXGC_FindBestVisual(JNIEnv *env, jint screen);

#endif /* HEADLESS */

#endif /* GLXGraphicsConfig_h_Included */
