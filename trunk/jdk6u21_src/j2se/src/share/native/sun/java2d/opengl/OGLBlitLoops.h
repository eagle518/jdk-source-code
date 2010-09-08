/*
 * @(#)OGLBlitLoops.h	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLBlitLoops_h_Included
#define OGLBlitLoops_h_Included

#include "sun_java2d_opengl_OGLBlitLoops.h"
#include "OGLSurfaceData.h"
#include "OGLContext.h"

#define OFFSET_SRCTYPE sun_java2d_opengl_OGLBlitLoops_OFFSET_SRCTYPE
#define OFFSET_HINT    sun_java2d_opengl_OGLBlitLoops_OFFSET_HINT
#define OFFSET_TEXTURE sun_java2d_opengl_OGLBlitLoops_OFFSET_TEXTURE
#define OFFSET_RTT     sun_java2d_opengl_OGLBlitLoops_OFFSET_RTT
#define OFFSET_XFORM   sun_java2d_opengl_OGLBlitLoops_OFFSET_XFORM
#define OFFSET_ISOBLIT sun_java2d_opengl_OGLBlitLoops_OFFSET_ISOBLIT

void OGLBlitLoops_IsoBlit(JNIEnv *env,
                          OGLContext *oglc, jlong pSrcOps, jlong pDstOps,
                          jboolean xform, jint hint,
                          jboolean texture, jboolean rtt,
                          jint sx1, jint sy1,
                          jint sx2, jint sy2,
                          jdouble dx1, jdouble dy1,
                          jdouble dx2, jdouble dy2);

void OGLBlitLoops_Blit(JNIEnv *env,
                       OGLContext *oglc, jlong pSrcOps, jlong pDstOps,
                       jboolean xform, jint hint,
                       jint srctype, jboolean texture,
                       jint sx1, jint sy1,
                       jint sx2, jint sy2,
                       jdouble dx1, jdouble dy1,
                       jdouble dx2, jdouble dy2);

void OGLBlitLoops_SurfaceToSwBlit(JNIEnv *env, OGLContext *oglc,
                                  jlong pSrcOps, jlong pDstOps, jint dsttype,
                                  jint srcx, jint srcy,
                                  jint dstx, jint dsty,
                                  jint width, jint height);

void OGLBlitLoops_CopyArea(JNIEnv *env,
                           OGLContext *oglc, OGLSDOps *dstOps,
                           jint x, jint y,
                           jint width, jint height,
                           jint dx, jint dy);

#endif /* OGLBlitLoops_h_Included */
