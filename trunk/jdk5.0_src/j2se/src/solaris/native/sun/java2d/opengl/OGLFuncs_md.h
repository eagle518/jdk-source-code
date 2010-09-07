/*
 * @(#)OGLFuncs_md.h	1.3 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLFuncs_md_h_Included
#define OGLFuncs_md_h_Included

#include <link.h>
#include "J2D_GL/glx.h"
#include "OGLFuncMacros.h"

/**
 * GLX 1.2 functions
 */
typedef void (GLAPIENTRY *glXDestroyContextType)(Display *dpy, GLXContext ctx);
typedef GLXContext (GLAPIENTRY *glXGetCurrentContextType)(void);
typedef GLXDrawable (GLAPIENTRY *glXGetCurrentDrawableType)(void);
typedef Bool (GLAPIENTRY *glXIsDirectType)(Display *dpy, GLXContext ctx);
typedef Bool (GLAPIENTRY *glXQueryExtensionType)(Display *dpy, int *errorBase, int *eventBase);
typedef Bool (GLAPIENTRY *glXQueryVersionType)(Display *dpy, int *major, int *minor);
typedef void (GLAPIENTRY *glXSwapBuffersType)(Display *dpy, GLXDrawable drawable);
typedef const char * (GLAPIENTRY *glXGetClientStringType)(Display *dpy, int name);
typedef const char * (GLAPIENTRY *glXQueryServerStringType)(Display *dpy, int screen, int name);
typedef const char * (GLAPIENTRY *glXQueryExtensionsStringType)(Display *dpy, int screen);
typedef void (GLAPIENTRY *glXWaitGLType)(void);

/**
 * GLX 1.3 functions
 */
typedef GLXFBConfig * (GLAPIENTRY *glXGetFBConfigsType)(Display *dpy, int screen, int *nelements);
typedef GLXFBConfig * (GLAPIENTRY *glXChooseFBConfigType)(Display *dpy, int screen, const int *attrib_list, int *nelements);
typedef int (GLAPIENTRY *glXGetFBConfigAttribType)(Display *dpy, GLXFBConfig  config, int attribute, int *value);
typedef XVisualInfo * (GLAPIENTRY *glXGetVisualFromFBConfigType)(Display *dpy, GLXFBConfig  config);
typedef GLXWindow (GLAPIENTRY *glXCreateWindowType)(Display *dpy, GLXFBConfig config, Window win, const int *attrib_list);
typedef void (GLAPIENTRY *glXDestroyWindowType)(Display *dpy, GLXWindow win);
typedef GLXPixmap (GLAPIENTRY *glXCreatePixmapType)(Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attrib_list);
typedef void (GLAPIENTRY *glXDestroyPixmapType)(Display *dpy, GLXPixmap pixmap);
typedef GLXPbuffer (GLAPIENTRY *glXCreatePbufferType)(Display *dpy, GLXFBConfig config, const int *attrib_list);
typedef void (GLAPIENTRY *glXDestroyPbufferType)(Display *dpy, GLXPbuffer pbuffer);
typedef void (GLAPIENTRY *glXQueryDrawableType)(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value);
typedef GLXContext (GLAPIENTRY *glXCreateNewContextType)(Display *dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
typedef Bool (GLAPIENTRY *glXMakeContextCurrentType)(Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
typedef GLXDrawable (GLAPIENTRY *glXGetCurrentReadDrawableType)(void);
typedef int (GLAPIENTRY *glXQueryContextType)(Display *dpy, GLXContext ctx, int attribute, int *value);
typedef void (GLAPIENTRY *glXSelectEventType)(Display *dpy, GLXDrawable draw, unsigned long event_mask);
typedef void (GLAPIENTRY *glXGetSelectedEventType)(Display *dpy, GLXDrawable draw, unsigned long *event_mask);

#define OGL_LIB_HANDLE pLibGL
#define OGL_DECLARE_LIB_HANDLE() \
    static void *OGL_LIB_HANDLE = NULL
#define OGL_LIB_IS_UNINITIALIZED() \
    (OGL_LIB_HANDLE == NULL)
#define OGL_OPEN_LIB() \
    OGL_LIB_HANDLE = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL)
#define OGL_CLOSE_LIB() \
    dlclose(OGL_LIB_HANDLE)
#define OGL_GET_PROC_ADDRESS(f) \
    dlsym(OGL_LIB_HANDLE, #f)
#define OGL_GET_EXT_PROC_ADDRESS(f) \
    OGL_GET_PROC_ADDRESS(f)

#define OGL_EXPRESS_PLATFORM_FUNCS(action) \
    OGL_##action##_FUNC(glXDestroyContext); \
    OGL_##action##_FUNC(glXGetCurrentContext); \
    OGL_##action##_FUNC(glXGetCurrentDrawable); \
    OGL_##action##_FUNC(glXIsDirect); \
    OGL_##action##_FUNC(glXQueryExtension); \
    OGL_##action##_FUNC(glXQueryVersion); \
    OGL_##action##_FUNC(glXSwapBuffers); \
    OGL_##action##_FUNC(glXGetClientString); \
    OGL_##action##_FUNC(glXQueryServerString); \
    OGL_##action##_FUNC(glXQueryExtensionsString); \
    OGL_##action##_FUNC(glXWaitGL); \
    OGL_##action##_FUNC(glXGetFBConfigs); \
    OGL_##action##_FUNC(glXChooseFBConfig); \
    OGL_##action##_FUNC(glXGetFBConfigAttrib); \
    OGL_##action##_FUNC(glXGetVisualFromFBConfig); \
    OGL_##action##_FUNC(glXCreateWindow); \
    OGL_##action##_FUNC(glXDestroyWindow); \
    OGL_##action##_FUNC(glXCreatePixmap); \
    OGL_##action##_FUNC(glXDestroyPixmap); \
    OGL_##action##_FUNC(glXCreatePbuffer); \
    OGL_##action##_FUNC(glXDestroyPbuffer); \
    OGL_##action##_FUNC(glXQueryDrawable); \
    OGL_##action##_FUNC(glXCreateNewContext); \
    OGL_##action##_FUNC(glXMakeContextCurrent); \
    OGL_##action##_FUNC(glXGetCurrentReadDrawable); \
    OGL_##action##_FUNC(glXQueryContext); \
    OGL_##action##_FUNC(glXSelectEvent); \
    OGL_##action##_FUNC(glXGetSelectedEvent);

#define OGL_EXPRESS_PLATFORM_EXT_FUNCS(action)

#endif /* OGLFuncs_md_h_Included */
