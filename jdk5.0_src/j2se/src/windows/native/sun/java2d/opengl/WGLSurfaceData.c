/*
 * @(#)WGLSurfaceData.c	1.2 04/04/14
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>

#include "sun_java2d_opengl_WGLSurfaceData.h"

#include "jni.h"
#include "jlong.h"
#include "jni_util.h"
#include "WGLGraphicsConfig.h"
#include "WGLSurfaceData.h"

/**
 * The methods in this file implement the native windowing system specific
 * layer (WGL) for the OpenGL-based Java 2D pipeline.
 */

extern LockFunc                     OGLSD_Lock;
extern GetRasInfoFunc               OGLSD_GetRasInfo;
extern UnlockFunc                   OGLSD_Unlock;
extern DisposeFunc                  OGLSD_Dispose;

static jclass wglgcClass;
static jmethodID getSharedContextID;
static jobject oglContextLock;

jboolean needGLFlush = JNI_FALSE;

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_WGLSurfaceData_initIDs(JNIEnv *env, jclass wglsd,
                                              jclass wglgc, jobject lock)
{
    J2dTraceLn(J2D_TRACE_INFO, "in WGLSurfaceData_initIDs");

    wglgcClass = (*env)->NewGlobalRef(env, wglgc);
    getSharedContextID = (*env)->GetStaticMethodID(env, wglgc,
                                             "getThreadSharedContext", "()J");
    oglContextLock = (*env)->NewGlobalRef(env, lock);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_WGLSurfaceData_initOps(JNIEnv *env, jobject wglsd,
                                              jobject peer, 
                                              jobject graphicsConfig)
{
    OGLSDOps *oglsdo = (OGLSDOps *)SurfaceData_InitOps(env, wglsd,
                                                       sizeof(OGLSDOps));
    WGLSDOps *wglsdo = (WGLSDOps *)malloc(sizeof(WGLSDOps));

    J2dTraceLn(J2D_TRACE_INFO, "in WGLSurfaceData_initOps");

    if (wglsdo == NULL) {
	JNU_ThrowOutOfMemoryError(env, "creating native wgl ops");
        return;
    }

    oglsdo->privOps = wglsdo;

    oglsdo->sdOps.Lock               = OGLSD_Lock;
    oglsdo->sdOps.GetRasInfo         = OGLSD_GetRasInfo;
    oglsdo->sdOps.Unlock             = OGLSD_Unlock;
    oglsdo->sdOps.Dispose            = OGLSD_Dispose;
    oglsdo->sdOps.dirty              = JNI_FALSE;

    oglsdo->drawableType = OGLSD_UNDEFINED;
    oglsdo->activeBuffer = GL_FRONT;

    if (peer != NULL) {
        wglsdo->peer = (*env)->NewWeakGlobalRef(env, peer);
    }
}

/**
 * This function disposes of any native windowing system resources associated
 * with this surface.  For instance, if the given OGLSDOps is of type
 * OGLSD_PBUFFER, this method implementation will destroy the actual pbuffer
 * surface.  Returns SD_SUCCESS.
 */
jint
OGLSD_DisposeOGLSurface(JNIEnv *env, OGLSDOps *oglsdo)
{
    WGLSDOps *wglsdo = (WGLSDOps *)oglsdo->privOps;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLSD_DisposeOGLSurface");

    if (oglsdo->drawableType == OGLSD_PBUFFER) {
        if (wglsdo->drawable.pbuffer != 0) {
            if (wglsdo->pbufferDC != 0) {
                j2d_wglReleasePbufferDCARB(wglsdo->drawable.pbuffer,
                                           wglsdo->pbufferDC);
            }
            j2d_wglDestroyPbufferARB(wglsdo->drawable.pbuffer);
        }
    }

    return SD_SUCCESS;
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_WGLSurfaceData_initWindow
    (JNIEnv *env, jobject wglsd,
     jlong pData, jlong pPeerData, jlong pConfigData,
     jint xoff, jint yoff)
{
    PIXELFORMATDESCRIPTOR pfd;
    WGLGraphicsConfigInfo *wglInfo =
        (WGLGraphicsConfigInfo *)jlong_to_ptr(pConfigData);
    OGLSDOps *oglsdo = (OGLSDOps *)jlong_to_ptr(pData);
    WGLSDOps *wglsdo;
    HWND window;
    RECT wbounds;
    HDC hdc;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLSurfaceData_initWindow");

    if (wglInfo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "graphics config info is null");
        return JNI_FALSE;
    }

    if (oglsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
        return JNI_FALSE;
    }

    wglsdo = (WGLSDOps *)oglsdo->privOps;
    if (wglsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "wgl ops are null");
        return JNI_FALSE;
    }

    window = AwtComponent_GetHWnd(env, pPeerData);
    if (IsWindow(window) == FALSE) {
        JNU_ThrowNullPointerException(env, "disposed component");
        return JNI_FALSE;
    }

    GetWindowRect(window, &wbounds);

    hdc = GetDC(window);
    if (hdc == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "invalid hdc");
        return JNI_FALSE;
    }

    if (!SetPixelFormat(hdc, wglInfo->pixfmt, &pfd)) {
        J2dTraceLn(J2D_TRACE_ERROR, "error setting pixel format");
        ReleaseDC(window, hdc);
        return JNI_FALSE;
    }

    ReleaseDC(window, hdc);

    oglsdo->drawableType = OGLSD_WINDOW;
    oglsdo->xOffset = xoff;
    oglsdo->yOffset = yoff;
    oglsdo->width = wbounds.right - wbounds.left;
    oglsdo->height = wbounds.bottom - wbounds.top;
    wglsdo->drawable.window = window;
    wglsdo->pbufferDC = 0;

    J2dTraceLn2(J2D_TRACE_VERBOSE, "successfully created window: %d %d",
                oglsdo->width, oglsdo->height);

    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_WGLSurfaceData_initPbuffer
    (JNIEnv *env, jobject wglsd,
     jlong pCtx, jlong pData,
     jint width, jint height)
{
    int attrlist[] = {
        WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGB_ARB,
        WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,
        0
    };
    OGLSDOps *oglsdo = (OGLSDOps *)jlong_to_ptr(pData);
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    WGLSDOps *wglsdo;
    WGLCtxInfo *ctxinfo;
    HWND hwnd;
    HDC hdc, pbufferDC;
    HPBUFFERARB pbuffer;
    GLuint texID;
    int pow2Width, pow2Height;
    int actualWidth, actualHeight;

    J2dTraceLn2(J2D_TRACE_INFO, "in WGLSurfaceData_initPbuffer (w=%d h=%d)",
                width, height);

    if (oglsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
        return JNI_FALSE;
    }

    wglsdo = (WGLSDOps *)oglsdo->privOps;
    if (wglsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "wgl ops are null");
	return JNI_FALSE;
    }

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "context is null");
        return JNI_FALSE;
    }

    ctxinfo = (WGLCtxInfo *)oglc->ctxInfo;
    if (ctxinfo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "wgl context info is null");
        return JNI_FALSE;
    }

    // calculate the power-of-two dimensions of the actual pbuffer surface
    pow2Width = OGLSD_NextPowerOfTwo(width, 4096);
    pow2Height = OGLSD_NextPowerOfTwo(height, 4096);
    if (pow2Width == 0 || pow2Height == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "dimensions too large");
        return JNI_FALSE;
    }

    // create a scratch window
    hwnd = WGLGC_CreateScratchWindow(ctxinfo->configInfo->screen);
    if (hwnd == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create scratch window");
        return JNI_FALSE;
    }

    // get the HDC for the scratch window
    hdc = GetDC(hwnd);
    if (hdc == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not get dc for scratch window");
        DestroyWindow(hwnd);
        return JNI_FALSE;
    }

    pbuffer = j2d_wglCreatePbufferARB(hdc, ctxinfo->configInfo->pixfmt,
                                      pow2Width, pow2Height, attrlist);

    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    if (pbuffer == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create wgl pbuffer");
        return JNI_FALSE;
    }

    // REMIND: we get the DC for the pbuffer at creation time, and then
    //         release the DC when the pbuffer is disposed; this works
    //         around some problems in the ATI drivers related to doing
    //         a Get/ReleasePbufferDC() everytime we make a context current
    pbufferDC = j2d_wglGetPbufferDCARB(pbuffer);
    if (pbufferDC == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not get dc for pbuffer");
        j2d_wglDestroyPbufferARB(pbuffer);
        return JNI_FALSE;
    }

    // make sure the actual dimensions match those that we requested
    j2d_wglQueryPbufferARB(pbuffer, WGL_PBUFFER_WIDTH_ARB, &actualWidth);
    j2d_wglQueryPbufferARB(pbuffer, WGL_PBUFFER_HEIGHT_ARB, &actualHeight);

    if (actualWidth != pow2Width || actualHeight != pow2Height) {
        J2dTraceLn2(J2D_TRACE_ERROR,
                    "actual dimensions != requested (actual: w=%d h=%d)",
                    actualWidth, actualHeight);
        j2d_wglReleasePbufferDCARB(pbuffer, pbufferDC);
        j2d_wglDestroyPbufferARB(pbuffer);
        return JNI_FALSE;
    }

    j2d_glGenTextures(1, &texID);
    j2d_glBindTexture(GL_TEXTURE_2D, texID);

    oglsdo->drawableType = OGLSD_PBUFFER;
    // REMIND: may need to adjust for pow2W/H here...
    oglsdo->xOffset = 0;
    oglsdo->yOffset = 0;
    oglsdo->width = width;
    oglsdo->height = height;
    oglsdo->textureID = texID;
    oglsdo->textureWidth = pow2Width;
    oglsdo->textureHeight = pow2Height;
    oglsdo->isPremult = JNI_TRUE;
    wglsdo->drawable.pbuffer = pbuffer;
    wglsdo->pbufferDC = pbufferDC;

    return JNI_TRUE;
}

/**
 * This function invokes the native threadsafe locking mechanism to ensure
 * that any following OpenGL commands have synchronized access to the OpenGL
 * libraries/hardware.
 */
void
OGLSD_LockImpl(JNIEnv *env)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_LockImpl");

    if ((*env)->MonitorEnter(env, oglContextLock) != JNI_OK) {
        J2dTraceLn(J2D_TRACE_ERROR, "error entering monitor");
    }
}

/**
 * This function invokes the native threadsafe flushing and unlocking
 * mechanism to ensure the preceding OpenGL commands are completed before
 * the surface is unlocked in a synchronized manner.  See OGLSurfaceData.h
 * for more information on the recognized constant values for the flushFlag
 * parameter.
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
        j2d_glFlush();
        break;

    case OGLSD_FLUSH_NOW:
        // otherwise, we have no choice but to flush immediately (or else
        // any rendering not invoked from the JED thread could be left
        // unflushed)
        j2d_glFlush();
        // FALLTHROUGH

    case OGLSD_NO_FLUSH:
    default:
        break;
    }

    if ((*env)->MonitorExit(env, oglContextLock) != JNI_OK) {
        J2dTraceLn(J2D_TRACE_ERROR, "error exiting monitor");
    }
}

/**
 * This function is a wrapper for the native Toolkit thread flushing
 * mechanism (on Windows, we don't have an elaborate flush mechanism like
 * we have on Unix, so we just issue a glFlush() here; ideally we'd have
 * a mechanism that periodically issues flushes to avoid calling that method
 * too often).
 */
void
OGLSD_OutputFlush(JNIEnv *env)
{
    j2d_glFlush();
}

/**
 * Convenience method that calls WGLGraphicsConfig.getThreadSharedContext(),
 * which will fetch the shared context for the current thread, and make that
 * shared context current.  Returns a pointer to the native shared OGLContext.
 */
OGLContext *
OGLSD_GetSharedContext(JNIEnv *env)
{
    jlong oglc;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_GetSharedContext");

    oglc = (*env)->CallStaticLongMethod(env, wglgcClass,
                                        getSharedContextID);

    return (OGLContext *)jlong_to_ptr(oglc);
}
