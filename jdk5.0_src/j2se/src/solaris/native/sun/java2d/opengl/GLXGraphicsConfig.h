/*
 * @(#)GLXGraphicsConfig.h	1.6 04/01/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef GLXGraphicsConfig_h_Included
#define GLXGraphicsConfig_h_Included

#include "jni.h"
#include "J2D_GL/glx.h"
#include "OGLSurfaceData.h"

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
 *     GLXFBConfig fbconfig;
 * A handle used in many GLX methods for querying certain attributes of the
 * GraphicsConfig (visual), creating new GLXContexts, and creating
 * GLXDrawable surfaces (pbuffers, etc).  Each GraphicsConfig has one
 * associated GLXFBConfig.
 *
 *     OGLExtInfo extInfo;
 * This structure contains flags that indicate whether a particular
 * extension (or logical group of extensions) is available for this
 * GLXGraphicsConfig.
 *
 *     jboolean isDoubleBuffered;
 * If true, indicates that this GLXGraphicsConfig (visual) is double-buffered.
 */
typedef struct _GLXGraphicsConfigInfo {
    jint          screen;
    jint          visual;
    GLXFBConfig   fbconfig;
    OGLExtInfo    extInfo;
    jboolean      isDoubleBuffered;
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
 */
typedef struct _GLXCtxInfo {
    GLXContext  context;
    GLXFBConfig fbconfig;
} GLXCtxInfo;

OGLContext *
GLXGC_InitOGLContext(JNIEnv *env, GLXGraphicsConfigInfo *glxinfo,
                     GLXContext sharedctx, jboolean useDisposer);

#endif /* HEADLESS */

#endif /* GLXGraphicsConfig_h_Included */
