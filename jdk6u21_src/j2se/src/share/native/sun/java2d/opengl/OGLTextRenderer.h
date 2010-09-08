/*
 * @(#)OGLTextRenderer.h	1.5 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLTextRenderer_h_Included
#define OGLTextRenderer_h_Included

#include <jni.h>
#include <jlong.h>
#include "sun_java2d_pipe_BufferedTextPipe.h"
#include "OGLContext.h"
#include "OGLSurfaceData.h"

#define BYTES_PER_GLYPH_IMAGE \
    sun_java2d_pipe_BufferedTextPipe_BYTES_PER_GLYPH_IMAGE
#define BYTES_PER_GLYPH_POSITION \
    sun_java2d_pipe_BufferedTextPipe_BYTES_PER_GLYPH_POSITION
#define BYTES_PER_POSITIONED_GLYPH \
    (BYTES_PER_GLYPH_IMAGE + BYTES_PER_GLYPH_POSITION)

#define OFFSET_CONTRAST  sun_java2d_pipe_BufferedTextPipe_OFFSET_CONTRAST
#define OFFSET_RGBORDER  sun_java2d_pipe_BufferedTextPipe_OFFSET_RGBORDER
#define OFFSET_SUBPIXPOS sun_java2d_pipe_BufferedTextPipe_OFFSET_SUBPIXPOS
#define OFFSET_POSITIONS sun_java2d_pipe_BufferedTextPipe_OFFSET_POSITIONS

void OGLTR_EnableGlyphVertexCache(OGLContext *oglc);
void OGLTR_DisableGlyphVertexCache(OGLContext *oglc);

void OGLTR_DrawGlyphList(JNIEnv *env, OGLContext *oglc, OGLSDOps *dstOps,
                         jint totalGlyphs, jboolean usePositions,
                         jboolean subPixPos, jboolean rgbOrder,
                         jint lcdContrast,
                         jfloat glyphListOrigX, jfloat glyphListOrigY,
                         unsigned char *images, unsigned char *positions);

#endif /* OGLTextRenderer_h_Included */
