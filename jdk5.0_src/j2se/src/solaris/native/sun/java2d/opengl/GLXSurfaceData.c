/*
 * @(#)GLXSurfaceData.c	1.13 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <pthread.h>

#include "sun_java2d_opengl_GLXSurfaceData.h"

#include "jlong.h"
#include "GLXGraphicsConfig.h"
#include "GLXSurfaceData.h"
#include "awt_Component.h"
#include "awt_GraphicsEnv.h"

/**
 * The methods in this file implement the native windowing system specific
 * layer (GLX) for the OpenGL-based Java 2D pipeline.
 */

#ifndef HEADLESS

extern LockFunc                     OGLSD_Lock;
extern GetRasInfoFunc               OGLSD_GetRasInfo;
extern UnlockFunc                   OGLSD_Unlock;
extern DisposeFunc                  OGLSD_Dispose;

extern struct MComponentPeerIDs mComponentPeerIDs;
extern struct X11GraphicsConfigIDs x11GraphicsConfigIDs;

static jclass glxgcClass;
static jmethodID getSharedContextID;

jboolean needGLFlush = JNI_FALSE;
jboolean surfaceCreationFailed = JNI_FALSE;

#endif /* !HEADLESS */

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_GLXSurfaceData_initIDs(JNIEnv *env, jclass glxsd,
                                              jclass glxgc)
{
#ifndef HEADLESS
    J2dTraceLn(J2D_TRACE_INFO, "in GLXSurfaceData_initIDs");

    glxgcClass = (*env)->NewGlobalRef(env, glxgc);
    getSharedContextID = (*env)->GetStaticMethodID(env, glxgc,
                                             "getThreadSharedContext", "()J");
#endif /* HEADLESS */
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_GLXSurfaceData_initOps(JNIEnv *env, jobject glxsd,
                                              jobject peer, 
                                              jobject graphicsConfig)
{
#ifndef HEADLESS
    OGLSDOps *oglsdo = (OGLSDOps *)SurfaceData_InitOps(env, glxsd, 
                                                       sizeof(OGLSDOps));
    GLXSDOps *glxsdo = (GLXSDOps *)malloc(sizeof(GLXSDOps));

    J2dTraceLn(J2D_TRACE_INFO, "in GLXSurfaceData_initOps");

    if (glxsdo == NULL) {
	JNU_ThrowOutOfMemoryError(env, "creating native GLX ops");
        return;
    }

    oglsdo->privOps = glxsdo;

    oglsdo->sdOps.Lock               = OGLSD_Lock;
    oglsdo->sdOps.GetRasInfo         = OGLSD_GetRasInfo;
    oglsdo->sdOps.Unlock             = OGLSD_Unlock;
    oglsdo->sdOps.Dispose            = OGLSD_Dispose;
    oglsdo->sdOps.dirty              = JNI_FALSE;

    oglsdo->drawableType = OGLSD_UNDEFINED;
    oglsdo->activeBuffer = GL_FRONT;

#ifdef XAWT
    if (peer != NULL) {
        glxsdo->window = JNU_CallMethodByName(env, NULL, peer,
                                              "getContentWindow", "()J").j;
    } else {
        glxsdo->window = 0;
    }
#else
    if (peer != NULL) {
	struct ComponentData *cdata;
	cdata = (struct ComponentData *)
	    JNU_GetLongFieldAsPtr(env, peer, mComponentPeerIDs.pData);
	if (cdata == NULL) {
            free(glxsdo);
	    JNU_ThrowNullPointerException(env, "Component data missing");
	    return;
	}
	if (cdata->widget == NULL) {
            free(glxsdo);
	    JNU_ThrowInternalError(env, "Widget is NULL in initOps");
	    return;
	}
        glxsdo->widget = cdata->widget;
    } else {
	glxsdo->widget = NULL;
    }
#endif

    glxsdo->configData = (AwtGraphicsConfigDataPtr)
	JNU_GetLongFieldAsPtr(env, graphicsConfig, 
                              x11GraphicsConfigIDs.aData);
    if (glxsdo->configData == NULL) {
        free(glxsdo);
	JNU_ThrowNullPointerException(env, 
                                 "Native GraphicsConfig data block missing");
	return;
    }

    if (glxsdo->configData->glxInfo == NULL) {
        free(glxsdo);
	JNU_ThrowNullPointerException(env, "GLXGraphicsConfigInfo missing");
	return;
    }
#endif /* HEADLESS */
}

#ifndef HEADLESS

/**
 * This function disposes of any native windowing system resources associated
 * with this surface.  For instance, if the given OGLSDOps is of type
 * OGLSD_PBUFFER, this method implementation will destroy the actual pbuffer
 * surface (with GLX, the glXDestroyPbuffer() method would be invoked).
 * Returns SD_SUCCESS if the operation was successful; SD_FAILURE otherwise.
 */
jint
OGLSD_DisposeOGLSurface(JNIEnv *env, OGLSDOps *oglsdo)
{
    GLXSDOps *glxsdo = (GLXSDOps *)oglsdo->privOps;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_DisposeOGLSurface");

    if (oglsdo->drawableType == OGLSD_PIXMAP) {
        if (glxsdo->drawable != 0) {
            j2d_glXDestroyPixmap(awt_display, glxsdo->drawable);
        }
        if (glxsdo->xdrawable != 0) {
            XFreePixmap(awt_display, glxsdo->xdrawable);
        }
    } else if (oglsdo->drawableType == OGLSD_PBUFFER) {
        if (glxsdo->drawable != 0) {
            j2d_glXDestroyPbuffer(awt_display, glxsdo->drawable);
        }
    } else if (oglsdo->drawableType == OGLSD_WINDOW) {
        // X Window is free'd later by AWT code...
    }

    return SD_SUCCESS;
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_GLXSurfaceData_initWindow
    (JNIEnv *env, jobject glxsd,
     jlong pData)
{
    OGLSDOps *oglsdo = (OGLSDOps *)jlong_to_ptr(pData);
    GLXSDOps *glxsdo;
    Window window;
#ifdef XAWT
    XWindowAttributes attr;
#else
    Widget widget;
#endif

    J2dTraceLn(J2D_TRACE_INFO, "in GLXSurfaceData_initWindow");

    if (oglsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
        return JNI_FALSE;
    }

    glxsdo = (GLXSDOps *)oglsdo->privOps;
    if (glxsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "glx ops are null");
        return JNI_FALSE;
    }

#ifdef XAWT
    window = glxsdo->window;
    if (window == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "window is invalid");
        return JNI_FALSE;
    }

    XGetWindowAttributes(awt_display, window, &attr);
    oglsdo->width = attr.width;
    oglsdo->height = attr.height;
#else
    widget = glxsdo->widget;
    if (widget == NULL) {
        J2dTraceLn(J2D_TRACE_WARNING, "widget is null");
    }

    if (!XtIsRealized(widget)) {
        J2dTraceLn(J2D_TRACE_ERROR, "widget is unrealized");
        return JNI_FALSE;
    }

    window = XtWindow(widget);
    oglsdo->width = widget->core.width;
    oglsdo->height = widget->core.height;
#endif

    oglsdo->drawableType = OGLSD_WINDOW;
    oglsdo->xOffset = 0;
    oglsdo->yOffset = 0;
    glxsdo->drawable = window;
    glxsdo->xdrawable = window;

    J2dTraceLn2(J2D_TRACE_VERBOSE, "glx window (w=%d h=%d)",
                oglsdo->width, oglsdo->height);

    return JNI_TRUE;
}

static int
GLXSD_BadAllocXErrHandler(Display *display, XErrorEvent *xerr)
{
    int ret = 0;
    if (xerr->error_code == BadAlloc) {
	surfaceCreationFailed = JNI_TRUE;
    } else {
	ret = (*xerror_saved_handler)(display, xerr);
    }
    return ret;
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_GLXSurfaceData_initPbuffer
    (JNIEnv *env, jobject glxsd,
     jlong pCtx, jlong pData,
     jint width, jint height)
{
    OGLSDOps *oglsdo = (OGLSDOps *)jlong_to_ptr(pData);
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    GLXSDOps *glxsdo;
    GLXCtxInfo *ctxinfo;
    GLXPbuffer pbuffer;
    int attrlist[] = {GLX_PBUFFER_WIDTH, 0,
                      GLX_PBUFFER_HEIGHT, 0,
                      GLX_PRESERVED_CONTENTS, GL_FALSE, 0};

    J2dTraceLn2(J2D_TRACE_INFO, "in GLXSurfaceData_initPbuffer (w=%d h=%d)",
                width, height);

    if (oglsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
        return JNI_FALSE;
    }

    glxsdo = (GLXSDOps *)oglsdo->privOps;
    if (glxsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "glx ops are null");
	return JNI_FALSE;
    }

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "context is null");
        return JNI_FALSE;
    }

    ctxinfo = (GLXCtxInfo *)oglc->ctxInfo;
    if (ctxinfo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "glx context info is null");
        return JNI_FALSE;
    }

    attrlist[1] = width;
    attrlist[3] = height;

    surfaceCreationFailed = JNI_FALSE;
    EXEC_WITH_XERROR_HANDLER(
        GLXSD_BadAllocXErrHandler,
        pbuffer = j2d_glXCreatePbuffer(awt_display,
                                       ctxinfo->fbconfig, attrlist));
    if ((pbuffer == 0) || surfaceCreationFailed) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create glx pbuffer");
        return JNI_FALSE;
    }

    oglsdo->drawableType = OGLSD_PBUFFER;
    oglsdo->width = width;
    oglsdo->height = height;
    oglsdo->xOffset = 0;
    oglsdo->yOffset = 0;
    oglsdo->isPremult = JNI_TRUE;

    glxsdo->drawable = pbuffer;
    glxsdo->xdrawable = 0;

    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_GLXSurfaceData_initPixmap
    (JNIEnv *env, jobject glxsd,
     jlong pCtx, jlong pData,
     jint width, jint height, jint depth)
{
    OGLSDOps *oglsdo = (OGLSDOps *)jlong_to_ptr(pData);
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    GLXSDOps *glxsdo;
    GLXCtxInfo *ctxinfo;
    Pixmap pixmap;
    GLXPixmap glxpixmap;

    J2dTraceLn2(J2D_TRACE_INFO, "in GLXSD_initPixmap (w=%d h=%d)",
                width, height);

    if (oglsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
        return JNI_FALSE;
    }

    glxsdo = (GLXSDOps *)oglsdo->privOps;
    if (glxsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "glx ops are null");
	return JNI_FALSE;
    }

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "context is null");
        return JNI_FALSE;
    }

    ctxinfo = (GLXCtxInfo *)oglc->ctxInfo;
    if (ctxinfo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "glx context info is null");
        return JNI_FALSE;
    }

    pixmap = XCreatePixmap(awt_display,
                           RootWindow(awt_display,  
                                      glxsdo->configData->awt_visInfo.screen),
                           width, height, depth);
    if (pixmap == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create pixmap");
        return JNI_FALSE;
    }

    glxpixmap = j2d_glXCreatePixmap(awt_display, ctxinfo->fbconfig,
                                    pixmap, NULL);
    if (glxpixmap == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create glx pixmap");
        XFreePixmap(awt_display, pixmap);
        return JNI_FALSE;
    }

    oglsdo->drawableType = OGLSD_PIXMAP;
    oglsdo->width = width;
    oglsdo->height = height;
    oglsdo->xOffset = 0;
    oglsdo->yOffset = 0;
    oglsdo->isPremult = JNI_TRUE;

    glxsdo->drawable = glxpixmap;
    glxsdo->xdrawable = pixmap;

    return JNI_TRUE;
}

/**
 * This function invokes the native threadsafe locking mechanism (e.g.
 * the AWT_LOCK() macro in the GLX case) to ensure that any following OpenGL
 * commands have synchronized access to the OpenGL libraries/hardware.
 */
void
OGLSD_LockImpl(JNIEnv *env)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_LockImpl");

    AWT_LOCK();
}

/**
 * This function invokes the native threadsafe flushing and unlocking
 * mechanism (e.g. the AWT_FLUSH_UNLOCK() macro in the GLX case) to ensure
 * the preceding OpenGL commands are completed before the surface is unlocked
 * in a synchronized manner.  See OGLSurfaceData.h for more information
 * on the recognized constant values for the flushFlag parameter.
 */
void
OGLSD_UnlockImpl(JNIEnv *env, jint flushFlag)
{
    J2dTraceLn1(J2D_TRACE_INFO, "in OGLSD_UnlockImpl (flush=%d)", flushFlag);

    switch (flushFlag) {
    case OGLSD_FLUSH_ON_JED:
        // if we're on the JED thread, we can use the native Toolkit thread
        // flushing mechanism, which will post an asynchronous event that will
        // eventually flush the context that is current to this thread
        needGLFlush = JNI_TRUE;
        AWT_FLUSH_UNLOCK();
        break;

    case OGLSD_FLUSH_NOW:
        // otherwise, we have no choice but to flush immediately (or else
        // any rendering not invoked from the JED thread could be left
        // unflushed)
        j2d_glFlush();
        // FALLTHROUGH

    case OGLSD_NO_FLUSH:
    default:
        AWT_UNLOCK();
        break;
    }
}

/**
 * This function is a wrapper around the awt_output_flush() method, which
 * is the native Toolkit thread flushing mechanism.
 */
void
OGLSD_OutputFlush(JNIEnv *env)
{
    awt_output_flush();
}

/**
 * Convenience method that calls GLXGraphicsConfig.getThreadSharedContext(),
 * which will fetch the shared context for the current thread, and make that
 * shared context current.  Returns a pointer to the native shared OGLContext.
 */
OGLContext *
OGLSD_GetSharedContext(JNIEnv *env)
{
    jlong oglc;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_GetSharedContext");

    oglc = (*env)->CallStaticLongMethod(env, glxgcClass,
                                        getSharedContextID);

    return (OGLContext *)jlong_to_ptr(oglc);
}

#endif /* !HEADLESS */
