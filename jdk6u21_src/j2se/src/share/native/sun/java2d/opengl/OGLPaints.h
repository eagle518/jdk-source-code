/*
 * @(#)OGLPaints.h	1.2 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLPaints_h_Included
#define OGLPaints_h_Included

#include "OGLContext.h"

void OGLPaints_ResetPaint(OGLContext *oglc);

void OGLPaints_SetColor(OGLContext *oglc, jint pixel);

void OGLPaints_SetGradientPaint(OGLContext *oglc,
                                jboolean useMask, jboolean cyclic,
                                jdouble p0, jdouble p1, jdouble p3,
                                jint pixel1, jint pixel2);

void OGLPaints_SetLinearGradientPaint(OGLContext *oglc, OGLSDOps *dstOps,
                                      jboolean useMask, jboolean linear,
                                      jint cycleMethod, jint numStops,
                                      jfloat p0, jfloat p1, jfloat p3,
                                      void *fractions, void *pixels);

void OGLPaints_SetRadialGradientPaint(OGLContext *oglc, OGLSDOps *dstOps,
                                      jboolean useMask, jboolean linear,
                                      jint cycleMethod, jint numStops,
                                      jfloat m00, jfloat m01, jfloat m02,
                                      jfloat m10, jfloat m11, jfloat m12,
                                      jfloat focusX,
                                      void *fractions, void *pixels);

void OGLPaints_SetTexturePaint(OGLContext *oglc,
                               jboolean useMask,
                               jlong pSrcOps, jboolean filter,
                               jdouble xp0, jdouble xp1, jdouble xp3,
                               jdouble yp0, jdouble yp1, jdouble yp3);

#endif /* OGLPaints_h_Included */
