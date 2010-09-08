/*
 * @(#)OGLMaskBlit.c	1.15 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef HEADLESS

#include <stdlib.h>
#include <jlong.h>

#include "OGLMaskBlit.h"
#include "OGLRenderQueue.h"
#include "OGLSurfaceData.h"

/**
 * REMIND: This method assumes that the dimensions of the incoming pixel
 *         array are less than or equal to the cached blit texture tile;
 *         these are rather fragile assumptions, and should be cleaned up...
 */
void
OGLMaskBlit_MaskBlit(JNIEnv *env, OGLContext *oglc,
                     jint dstx, jint dsty,
                     jint width, jint height,
                     void *pPixels)
{
    GLfloat tx1, ty1, tx2, ty2;

    J2dTraceLn(J2D_TRACE_INFO, "OGLMaskBlit_MaskBlit");

    if (width <= 0 || height <= 0) {
        J2dTraceLn(J2D_TRACE_WARNING,
                   "OGLMaskBlit_MaskBlit: invalid dimensions");
        return;
    }

    RETURN_IF_NULL(pPixels);
    RETURN_IF_NULL(oglc);
    CHECK_PREVIOUS_OP(GL_TEXTURE_2D);

    if (oglc->blitTextureID == 0) {
        if (!OGLContext_InitBlitTileTexture(oglc)) {
            J2dRlsTraceLn(J2D_TRACE_ERROR,
                "OGLMaskBlit_MaskBlit: could not init blit tile");
            return;
        }
    }

    // set up texture parameters
    j2d_glBindTexture(GL_TEXTURE_2D, oglc->blitTextureID);
    OGLC_UPDATE_TEXTURE_FUNCTION(oglc, GL_MODULATE);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // copy system memory IntArgbPre surface into cached texture
    j2d_glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0, width, height,
                        GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pPixels);

    tx1 = 0.0f;
    ty1 = 0.0f;
    tx2 = ((GLfloat)width) / OGLC_BLIT_TILE_SIZE;
    ty2 = ((GLfloat)height) / OGLC_BLIT_TILE_SIZE;

    // render cached texture to the OpenGL surface
    j2d_glBegin(GL_QUADS);
    j2d_glTexCoord2f(tx1, ty1); j2d_glVertex2i(dstx, dsty);
    j2d_glTexCoord2f(tx2, ty1); j2d_glVertex2i(dstx + width, dsty);
    j2d_glTexCoord2f(tx2, ty2); j2d_glVertex2i(dstx + width, dsty + height);
    j2d_glTexCoord2f(tx1, ty2); j2d_glVertex2i(dstx, dsty + height);
    j2d_glEnd();
}

#endif /* !HEADLESS */
