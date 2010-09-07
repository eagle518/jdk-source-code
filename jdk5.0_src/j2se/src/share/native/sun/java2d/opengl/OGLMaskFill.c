/*
 * @(#)OGLMaskFill.c	1.6 04/03/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef HEADLESS

#include <stdlib.h>
#include <jlong.h>

#include "sun_java2d_opengl_OGLMaskFill.h"

#include "OGLSurfaceData.h"
#include "GraphicsPrimitiveMgr.h"

/**
 * This implementation first copies the alpha tile into a texture and then
 * maps that texture to the destination surface.  This approach appears to
 * offer the best performance despite being a two-step process.
 *
 * Here are some descriptions of the many variables used in this method:
 *   x,y     - upper left corner of the tile destination
 *   w,h     - width/height of the mask tile
 *   x0      - placekeeper for the original destination x location
 *   tx1,ty1 - upper left corner of the texture tile source region (always 0)
 *   tx2,ty2 - lower right corner of the texture tile source region
 *             in the range [0.0,1.0]
 *   tw,th   - width/height of the actual texture tile in pixels
 *   sx1,sy1 - upper left corner of the mask tile source region
 *   sx2,sy2 - lower left corner of the mask tile source region
 *   sx,sy   - "current" upper left corner of the mask tile region of interest
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLMaskFill_MaskFill
    (JNIEnv *env, jobject self,
     jlong pCtx,
     jint x, jint y, jint w, jint h,
     jbyteArray maskArray,
     jint maskoff, jint maskscan)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLMaskFill_MaskFill");
    J2dTraceLn4(J2D_TRACE_VERBOSE, "x=%d y=%d w=%d h=%d", x, y, w, h);
    J2dTraceLn2(J2D_TRACE_VERBOSE, "maskoff=%d maskscan=%d",
                maskoff, maskscan);

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "context is null");
        return;
    }

    if (oglc->maskTextureID == 0) {
        if (OGLSD_InitMaskTileTexture(oglc) == SD_FAILURE) {
            return;
        }
    }

    if (maskArray) {
        unsigned char *pMask = 
            (*env)->GetPrimitiveArrayCritical(env, maskArray, 0);
        GLfloat tx1, ty1, tx2, ty2;
        jint tw, th, x0;
        jint sx1, sy1, sx2, sy2;
        jint sx, sy, sw, sh;

        if (pMask == NULL) {
            return;
        }

        x0 = x;
        tx1 = 0.0f;
        ty1 = 0.0f;
        tw = OGLSD_MASK_TILE_SIZE;
        th = OGLSD_MASK_TILE_SIZE;
        sx1 = maskoff % maskscan;
        sy1 = maskoff / maskscan;
        sx2 = sx1 + w;
        sy2 = sy1 + h;

        j2d_glEnable(GL_TEXTURE_2D);
        j2d_glBindTexture(GL_TEXTURE_2D, oglc->maskTextureID);
        j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, maskscan);
        j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (sy = sy1; sy < sy2; sy += th, y += th) {
            x = x0;
            sh = ((sy + th) > sy2) ? (sy2 - sy) : th;

            for (sx = sx1; sx < sx2; sx += tw, x += tw) {
                sw = ((sx + tw) > sx2) ? (sx2 - sx) : tw;

                // update the source pointer offsets
                j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, sx);
                j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, sy);

                // copy alpha mask into texture tile
                j2d_glTexSubImage2D(GL_TEXTURE_2D, 0,
                                    0, 0, sw, sh,
                                    GL_LUMINANCE, GL_UNSIGNED_BYTE, pMask);

                // update the lower right texture coordinates
                tx2 = ((GLfloat)sw) / tw;
                ty2 = ((GLfloat)sh) / th;

                // render texture tile to the destination surface
                j2d_glBegin(GL_QUADS);
                j2d_glTexCoord2f(tx1, ty1); j2d_glVertex2i(x, y);
                j2d_glTexCoord2f(tx2, ty1); j2d_glVertex2i(x + sw, y);
                j2d_glTexCoord2f(tx2, ty2); j2d_glVertex2i(x + sw, y + sh);
                j2d_glTexCoord2f(tx1, ty2); j2d_glVertex2i(x, y + sh);
                j2d_glEnd();
            }
        }

        j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        j2d_glDisable(GL_TEXTURE_2D);

        (*env)->ReleasePrimitiveArrayCritical(env, maskArray,
                                              pMask, JNI_ABORT);
    } else {
        GLRECT(x, y, w, h);
    }

    OGLContext_Flush(env, oglc);
}

#endif /* !HEADLESS */
