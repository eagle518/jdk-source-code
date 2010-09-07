/*
 * @(#)GLXContext.c	1.1 04/01/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef HEADLESS

#include <jlong.h>

#include "GLXGraphicsConfig.h"
#include "GLXSurfaceData.h"
#include "awt_p.h"

extern OGLContext *sharedContext;

JNIEXPORT jlong JNICALL
Java_sun_java2d_opengl_GLXContext_initNativeContext(JNIEnv *env, jobject glxc,
                                                    jlong aData)
{
    AwtGraphicsConfigDataPtr configData =
        (AwtGraphicsConfigDataPtr)jlong_to_ptr(aData);
    GLXCtxInfo *sharedInfo = (GLXCtxInfo *)sharedContext->ctxInfo;
    OGLContext *oglc;

    J2dTraceLn(J2D_TRACE_INFO, "in GLXContext_initNativeContext");

    oglc = GLXGC_InitOGLContext(env, configData->glxInfo,
                                sharedInfo->context, JNI_TRUE);

    return ptr_to_jlong(oglc);
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_GLXContext_makeNativeContextCurrent(JNIEnv *env,
                                                           jobject glxc,
                                                           jlong pCtx,
                                                           jlong pSrc,
                                                           jlong pDst)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    OGLSDOps *srcOps = (OGLSDOps *)jlong_to_ptr(pSrc);
    OGLSDOps *dstOps = (OGLSDOps *)jlong_to_ptr(pDst);
    GLXCtxInfo *ctxinfo;
    GLXSDOps *srcGLXOps;
    GLXSDOps *dstGLXOps;

    J2dTraceLn(J2D_TRACE_INFO, "in GLXContext_makeNativeContextCurrent");

    if (srcOps == NULL || dstOps == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
        return JNI_FALSE;
    }

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "context is null");
        return JNI_FALSE;
    }

    ctxinfo = (GLXCtxInfo *)oglc->ctxInfo;
    srcGLXOps = (GLXSDOps *)srcOps->privOps;
    dstGLXOps = (GLXSDOps *)dstOps->privOps;

    if (!j2d_glXMakeContextCurrent(awt_display,
                                   dstGLXOps->drawable, srcGLXOps->drawable,
                                   ctxinfo->context))
    {
        J2dTraceLn(J2D_TRACE_ERROR, "could not make GLX context current");
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

#endif /* !HEADLESS */
