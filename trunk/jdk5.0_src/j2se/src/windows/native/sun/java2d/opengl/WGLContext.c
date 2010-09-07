/*
 * @(#)WGLContext.c	1.2 04/04/14
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jlong.h>

#include "WGLGraphicsConfig.h"
#include "WGLSurfaceData.h"

JNIEXPORT jlong JNICALL
Java_sun_java2d_opengl_WGLContext_initNativeContext(JNIEnv *env, jobject wglc,
                                                    jlong pInfo)
{
    WGLGraphicsConfigInfo *wglInfo =
        (WGLGraphicsConfigInfo *)jlong_to_ptr(pInfo);
    OGLContext *oglc;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLContext_initNativeContext");

    if (wglInfo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "graphics config info is null");
        return 0L;
    }

    oglc = WGLGC_InitOGLContext(env, wglInfo, JNI_TRUE);

    return ptr_to_jlong(oglc);
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_WGLContext_makeNativeContextCurrent(JNIEnv *env,
                                                           jobject wglc,
                                                           jlong pCtx,
                                                           jlong pSrc,
                                                           jlong pDst)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    OGLSDOps *srcOps = (OGLSDOps *)jlong_to_ptr(pSrc);
    OGLSDOps *dstOps = (OGLSDOps *)jlong_to_ptr(pDst);
    WGLCtxInfo *ctxinfo;
    WGLSDOps *srcWGLOps;
    WGLSDOps *dstWGLOps;
    HDC dstHDC;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLContext_makeNativeContextCurrent");

    if (srcOps == NULL || dstOps == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
        return JNI_FALSE;
    }

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "context is null");
        return JNI_FALSE;
    }

    ctxinfo = (WGLCtxInfo *)oglc->ctxInfo;
    srcWGLOps = (WGLSDOps *)srcOps->privOps;
    dstWGLOps = (WGLSDOps *)dstOps->privOps;

    J2dTraceLn4(J2D_TRACE_VERBOSE, "src: %d %p dst: %d %p",
                srcOps->drawableType, srcOps,
                dstOps->drawableType, dstOps);

    // get the hdc for the destination surface
    if (dstOps->drawableType == OGLSD_PBUFFER) {
        dstHDC = dstWGLOps->pbufferDC;
    } else {
        dstHDC = GetDC(dstWGLOps->drawable.window);
    }

    // make the context current to the destination hdc
    if (!j2d_wglMakeCurrent(dstHDC, ctxinfo->context)) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not make WGL context current");
        if (dstOps->drawableType != OGLSD_PBUFFER) {
            ReleaseDC(dstWGLOps->drawable.window, dstHDC);
        }
        return JNI_FALSE;
    }

    // release the hdc
    if (dstOps->drawableType == OGLSD_PBUFFER) {
        // release pbuffer from the render texture object (since we are
        // preparing to render to the pbuffer)
        j2d_glBindTexture(GL_TEXTURE_2D, dstOps->textureID);
        j2d_wglReleaseTexImageARB(dstWGLOps->drawable.pbuffer,
                                  WGL_FRONT_LEFT_ARB);
    } else {
        ReleaseDC(dstWGLOps->drawable.window, dstHDC);
    }

    if ((srcOps != dstOps) &&
        (srcOps->drawableType == OGLSD_PBUFFER))
    {
        // bind pbuffer to the render texture object (since we are preparing
        // to copy from the pbuffer)
        j2d_glBindTexture(GL_TEXTURE_2D, srcOps->textureID);
        j2d_wglBindTexImageARB(srcWGLOps->drawable.pbuffer,
                               WGL_FRONT_LEFT_ARB);
    }

    return JNI_TRUE;
}
