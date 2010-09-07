/*
 * @(#)OGLRenderer.c	1.7 04/03/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef HEADLESS

#include <jlong.h>
#include <jni_util.h>

#include "sun_java2d_opengl_OGLRenderer.h"

#include "OGLBlitLoops.h"
#include "OGLSurfaceData.h"
#include "SpanIterator.h"

static GLuint gradientTexID = 0;

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLRenderer_doDrawLine
    (JNIEnv *env, jobject oglr,
     jlong pCtx, jint x1, jint y1, jint x2, jint y2)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_doDrawLine");

    if (x1 == x2 || y1 == y2) {
        if (x1 > x2) {
            jint t = x1; x1 = x2; x2 = t;
        }
        if (y1 > y2) {
            jint t = y1; y1 = y2; y2 = t;
        }
        GLRECT(x1, y1, x2-x1+1, y2-y1+1);
    } else {
        GLfloat fx1 = (GLfloat)x1;
        GLfloat fy1 = (GLfloat)y1;
        GLfloat fx2 = (GLfloat)x2;
        GLfloat fy2 = (GLfloat)y2;

        if (x1 < x2) {
            fx1 += 0.2f;
            fx2 += 0.8f;
        } else {
            fx1 += 0.8f;
            fx2 += 0.2f;
        }

        if (y1 < y2) {
            fy1 += 0.2f;
            fy2 += 0.8f;
        } else {
            fy1 += 0.8f;
            fy2 += 0.2f;
        }

        j2d_glBegin(GL_LINES);
        j2d_glVertex2f(fx1, fy1);
        j2d_glVertex2f(fx2, fy2);
        j2d_glEnd();
    }

    OGLContext_Flush(env, oglc);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLRenderer_doDrawRect
    (JNIEnv *env, jobject oglr,
     jlong pCtx, jint x, jint y, jint w, jint h)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_doDrawRect");

    if (w < 0 || h < 0) {
        return;
    }

    if (w < 2 || h < 2) {
        // If one dimension is less than 2 then there is no
        // gap in the middle - draw a solid filled rectangle.
        GLRECT(x, y, w+1, h+1);
    } else {
        // Avoid drawing the endpoints twice.
        // Also prefer including the endpoints in the
        // horizontal sections which draw pixels faster.
        GLRECT_BEGIN;
        GLRECT_BODY_XYWH( x,   y,  w+1,  1 );
        GLRECT_BODY_XYWH( x,  y+1,  1,  h-1);
        GLRECT_BODY_XYWH(x+w, y+1,  1,  h-1);
        GLRECT_BODY_XYWH( x,  y+h, w+1,  1 );
        GLRECT_END;
    }

    OGLContext_Flush(env, oglc);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLRenderer_doDrawPoly
    (JNIEnv *env, jobject oglr,
     jlong pCtx, jint transx, jint transy,
     jintArray xcoordsArray, jintArray ycoordsArray,
     jint npoints, jboolean isclosed)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    jint *xcoords, *ycoords;
    jint i;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_doDrawPoly");

    if (JNU_IsNull(env, xcoordsArray) || JNU_IsNull(env, ycoordsArray)) {
        JNU_ThrowNullPointerException(env, "coordinate array");
        return;
    }
    if ((*env)->GetArrayLength(env, ycoordsArray) < npoints ||
        (*env)->GetArrayLength(env, xcoordsArray) < npoints)
    {
        JNU_ThrowArrayIndexOutOfBoundsException(env, "coordinate array");
        return;
    }

    xcoords = (jint *)
        (*env)->GetPrimitiveArrayCritical(env, xcoordsArray, NULL);
    if (xcoords == NULL) {
        return;
    }
    
    ycoords = (jint *)
        (*env)->GetPrimitiveArrayCritical(env, ycoordsArray, NULL);
    if (ycoords == NULL) {
        (*env)->ReleasePrimitiveArrayCritical(env, xcoordsArray, xcoords,
                                              JNI_ABORT);
        return;
    }

    // REMIND: this approach isn't correct (missing end points, etc)...
    j2d_glBegin(isclosed ? GL_LINE_LOOP : GL_LINE_STRIP);
    for (i = 0; i < npoints; i++) {
        j2d_glVertex2i(xcoords[i] + transx, ycoords[i] + transy);
    }
    j2d_glEnd();

    (*env)->ReleasePrimitiveArrayCritical(env, xcoordsArray, xcoords,
                                          JNI_ABORT);
    (*env)->ReleasePrimitiveArrayCritical(env, ycoordsArray, ycoords,
                                          JNI_ABORT);

    OGLContext_Flush(env, oglc);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLRenderer_doFillRect
    (JNIEnv *env, jobject oglr,
     jlong pCtx, jint x, jint y, jint w, jint h)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_doFillRect");

    if (w <= 0 || h <= 0) {
        return;
    }

    GLRECT(x, y, w, h);

    OGLContext_Flush(env, oglc);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLRenderer_devFillSpans
    (JNIEnv *env, jobject oglr,
     jlong pCtx, jobject si, jlong pIterator,
     jint transx, jint transy)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    SpanIteratorFuncs *pFuncs = (SpanIteratorFuncs *)jlong_to_ptr(pIterator);
    void *srData;
    jint x, y, w, h;
    jint spanbox[4];

    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_doFillSpans");

    if (JNU_IsNull(env, si)) {
        JNU_ThrowNullPointerException(env, "span iterator");
        return;
    }
    if (pFuncs == NULL) {
        JNU_ThrowNullPointerException(env, "native iterator not supplied");
        return;
    }

    srData = (*pFuncs->open)(env, si);
    GLRECT_BEGIN;
    while ((*pFuncs->nextSpan)(srData, spanbox)) {
        x = spanbox[0] + transx;
        y = spanbox[1] + transy;
        w = spanbox[2] - spanbox[0];
        h = spanbox[3] - spanbox[1];
        GLRECT_BODY_XYWH(x, y, w, h);
    }
    GLRECT_END;
    (*pFuncs->close)(env, srData);

    OGLContext_Flush(env, oglc);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLRenderer_devCopyArea
    (JNIEnv *env, jobject oglr,
     jlong pCtx, jlong pDst,
     jint srcx, jint srcy, jint dstx, jint dsty,
     jint width, jint height)
{
    OGLSDOps *dstOps = (OGLSDOps *)jlong_to_ptr(pDst);
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_devCopyArea");

    OGLBlitSurfaceToSurface(dstOps, dstOps,
                            srcx, srcy, dstx, dsty,
                            width, height, width, height);

    OGLContext_Flush(env, oglc);
}

static void
OGLRenderer_InitGradientTexture()
{
    GLclampf priority = 1.0f;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_InitGradientTexture");

    j2d_glGenTextures(1, &gradientTexID);
    j2d_glBindTexture(GL_TEXTURE_1D, gradientTexID);
    j2d_glPrioritizeTextures(1, &gradientTexID, &priority);
    j2d_glTexImage1D(GL_TEXTURE_1D, 0,
                     GL_RGBA, 4, 1, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLRenderer_enableGradientPaint
    (JNIEnv *env, jobject oglr,
     jlong pCtx,
     jboolean cyclic,
     jdouble p0, jdouble p1, jdouble p3,
     jint pixel1, jint pixel2)
{
    GLdouble texParams[4];
    GLuint pixels[4];

    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_enableGradientPaint");

    texParams[0] = p0;
    texParams[1] = p1;
    texParams[2] = 0.0;
    texParams[3] = p3;

    // repeat colors in texture border so that clamping interpolates properly
    pixels[0] = pixel1;
    pixels[1] = pixel1;
    pixels[2] = pixel2;
    pixels[3] = pixel2;

    if (gradientTexID == 0) {
        OGLRenderer_InitGradientTexture();
    }

    j2d_glEnable(GL_TEXTURE_1D);
    j2d_glEnable(GL_TEXTURE_GEN_S);
    j2d_glBindTexture(GL_TEXTURE_1D, gradientTexID);
    j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    j2d_glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    j2d_glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    j2d_glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S,
                        cyclic ? GL_REPEAT : GL_CLAMP);
    j2d_glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    j2d_glTexGendv(GL_S, GL_OBJECT_PLANE, texParams);

    j2d_glTexSubImage1D(GL_TEXTURE_1D, 0,
                        -1, 4, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLRenderer_disableGradientPaint
    (JNIEnv *env, jobject oglr,
     jlong pCtx)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_disableGradientPaint");

    j2d_glDisable(GL_TEXTURE_1D);
    j2d_glDisable(GL_TEXTURE_GEN_S);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLRenderer_enableTexturePaint
    (JNIEnv *env, jobject oglr,
     jlong pCtx, jlong pSrcOps, jboolean filter,
     jdouble xp0, jdouble xp1, jdouble xp3,
     jdouble yp0, jdouble yp1, jdouble yp3)
{
    OGLSDOps *srcOps = (OGLSDOps *)jlong_to_ptr(pSrcOps);
    GLdouble xParams[4];
    GLdouble yParams[4];
    GLint hint = (filter ? GL_LINEAR : GL_NEAREST);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_enableTexturePaint");

    if (srcOps == NULL) {
        return;
    }

    xParams[0] = xp0;
    xParams[1] = xp1;
    xParams[2] = 0.0;
    xParams[3] = xp3;

    yParams[0] = yp0;
    yParams[1] = yp1;
    yParams[2] = 0.0;
    yParams[3] = yp3;

    j2d_glEnable(GL_TEXTURE_2D);
    j2d_glEnable(GL_TEXTURE_GEN_S);
    j2d_glEnable(GL_TEXTURE_GEN_T);
    j2d_glBindTexture(GL_TEXTURE_2D, srcOps->textureID);
    j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, hint);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, hint);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    j2d_glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    j2d_glTexGendv(GL_S, GL_OBJECT_PLANE, xParams);
    j2d_glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    j2d_glTexGendv(GL_T, GL_OBJECT_PLANE, yParams);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLRenderer_disableTexturePaint
    (JNIEnv *env, jobject oglr,
     jlong pCtx)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLRenderer_disableTexturePaint");

    j2d_glDisable(GL_TEXTURE_2D);
    j2d_glDisable(GL_TEXTURE_GEN_S);
    j2d_glDisable(GL_TEXTURE_GEN_T);
}

#endif /* !HEADLESS */
