/*
 * @(#)OGLRenderer.h	1.6 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLRenderer_h_Included
#define OGLRenderer_h_Included

#include "sun_java2d_pipe_BufferedRenderPipe.h"
#include "OGLContext.h"

#define BYTES_PER_POLY_POINT \
    sun_java2d_pipe_BufferedRenderPipe_BYTES_PER_POLY_POINT
#define BYTES_PER_SCANLINE \
    sun_java2d_pipe_BufferedRenderPipe_BYTES_PER_SCANLINE
#define BYTES_PER_SPAN \
    sun_java2d_pipe_BufferedRenderPipe_BYTES_PER_SPAN

void OGLRenderer_DrawLine(OGLContext *oglc,
                          jint x1, jint y1, jint x2, jint y2);
void OGLRenderer_DrawRect(OGLContext *oglc,
                          jint x, jint y, jint w, jint h);
void OGLRenderer_DrawPoly(OGLContext *oglc,
                          jint nPoints, jint isClosed,
                          jint transX, jint transY,
                          jint *xPoints, jint *yPoints);
void OGLRenderer_DrawScanlines(OGLContext *oglc,
                               jint count, jint *scanlines);
void OGLRenderer_DrawParallelogram(OGLContext *oglc,
                                   jfloat fx11, jfloat fy11,
                                   jfloat dx21, jfloat dy21,
                                   jfloat dx12, jfloat dy12,
                                   jfloat lw21, jfloat lw12);
void OGLRenderer_DrawAAParallelogram(OGLContext *oglc, OGLSDOps *dstOps,
                                     jfloat fx11, jfloat fy11,
                                     jfloat dx21, jfloat dy21,
                                     jfloat dx12, jfloat dy12,
                                     jfloat lw21, jfloat lw12);

void OGLRenderer_FillRect(OGLContext *oglc,
                          jint x, jint y, jint w, jint h);
void OGLRenderer_FillSpans(OGLContext *oglc,
                           jint count, jint *spans);
void OGLRenderer_FillParallelogram(OGLContext *oglc,
                                   jfloat fx11, jfloat fy11,
                                   jfloat dx21, jfloat dy21,
                                   jfloat dx12, jfloat dy12);
void OGLRenderer_FillAAParallelogram(OGLContext *oglc, OGLSDOps *dstOps,
                                     jfloat fx11, jfloat fy11,
                                     jfloat dx21, jfloat dy21,
                                     jfloat dx12, jfloat dy12);

void OGLRenderer_EnableAAParallelogramProgram();
void OGLRenderer_DisableAAParallelogramProgram();

#endif /* OGLRenderer_h_Included */
