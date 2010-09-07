/*
 * @(#)OGLMaskBlit.c	1.8 04/03/31
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef HEADLESS

#include <stdlib.h>
#include <jlong.h>

#include "sun_java2d_opengl_OGLMaskBlit.h"

#include "OGLSurfaceData.h"
#include "GraphicsPrimitiveMgr.h"
#include "IntArgb.h"
#include "IntRgb.h"
#include "IntBgr.h"

#define MASK_LENGTH (OGLSD_BLIT_TILE_SIZE * OGLSD_BLIT_TILE_SIZE)

extern unsigned char mul8table[256][256];
extern OGLPixelFormat PixelFormats[];
static GLuint *rgbamask = NULL;

/**
 * This implementation of MaskBlit first combines the source system memory
 * tile with the corresponding alpha mask and stores the result in an
 * intermediate IntRgbaPre buffer.  That buffer is then copied into a cached
 * RGBA texture tile and then rendered to the destination surface.
 *
 * Note that currently there are only inner loops defined for IntArgb,
 * IntRgb, and IntBgr, as those are the most commonly used formats for this
 * operation.
 *
 * REMIND: this method assumes that rgbamask and the cached blit texture tile
 *         have the same dimensions, and that the incoming maskArray is
 *         of equal or lesser dimensions; these are rather fragile
 *         assumptions, and should be cleaned up at some point...
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLMaskBlit_MaskBlit
    (JNIEnv *env, jobject self,
     jlong pCtx, jlong pSrcOps,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height,
     jbyteArray maskArray,
     jint maskoff, jint maskscan,
     jint srctype)
{
    SurfaceDataOps *srcOps = (SurfaceDataOps *)jlong_to_ptr(pSrcOps);
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    SurfaceDataRasInfo srcInfo;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLMaskBlit_MaskBlit");

    if (width <= 0 || height <= 0) {
        J2dTraceLn(J2D_TRACE_WARNING, "invalid dimensions");
        return;
    }

    if (maskArray == NULL) {
        // should not get here; this case is handled at the Java level...
        J2dTraceLn(J2D_TRACE_WARNING, "maskArray is null");
        return;
    }

    if (srcOps == NULL) {
        J2dTraceLn(J2D_TRACE_WARNING, "ops are null");
	return;
    }

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_WARNING, "context is null");
	return;
    }

    if (rgbamask == NULL) {
        rgbamask = (GLuint *)malloc(sizeof(GLuint) * MASK_LENGTH);
        if (rgbamask == NULL) {
            J2dTraceLn(J2D_TRACE_ERROR, "could not allocate rgbamask");
            return;
        }
    }

    if (oglc->blitTextureID == 0) {
        if (OGLSD_InitBlitTileTexture(oglc) == SD_FAILURE) {
            J2dTraceLn(J2D_TRACE_ERROR, "could not init blit tile");
            return;
        }
    }

    srcInfo.bounds.x1 = srcx;
    srcInfo.bounds.y1 = srcy;
    srcInfo.bounds.x2 = srcx + width;
    srcInfo.bounds.y2 = srcy + height;

    if (srcOps->Lock(env, srcOps, &srcInfo, SD_LOCK_READ) != SD_SUCCESS) {
        J2dTraceLn(J2D_TRACE_WARNING, "could not acquire lock");
	return;
    }

    if (srcInfo.bounds.x2 > srcInfo.bounds.x1 &&
	srcInfo.bounds.y2 > srcInfo.bounds.y1)
    {
        srcOps->GetRasInfo(env, srcOps, &srcInfo);
	if (srcInfo.rasBase) {
            jfloat tx1, ty1, tx2, ty2;
            GLubyte ea;
            jint h;
            jint srcScanStride = srcInfo.scanStride;
            jint srcPixelStride = srcInfo.pixelStride;
            void *pSrcBase = PtrCoord(srcInfo.rasBase,
                                      srcInfo.bounds.x1, srcInfo.pixelStride,
                                      srcInfo.bounds.y1, srcInfo.scanStride);
            jint *pSrc = (jint *)pSrcBase;
            jsize masklen = (*env)->GetArrayLength(env, maskArray);
            unsigned char *pMaskBase =
                (*env)->GetPrimitiveArrayCritical(env, maskArray, 0);
            unsigned char *pMask = pMaskBase;
            jint newscan;
            GLuint *rgbavals;

            if (pMask == NULL) {
                J2dTraceLn(J2D_TRACE_ERROR, "could not lock mask array");
                SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
                SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
                return;
            }

            if (masklen > MASK_LENGTH) {
                // REMIND: this approach is seriously flawed if the mask
                //         length is ever greater than MASK_LENGTH (won't fit
                //         into the cached mask tile)...
                J2dTraceLn(J2D_TRACE_ERROR, "mask array too large (FIXME)");
                (*env)->ReleasePrimitiveArrayCritical(env, maskArray,
                                                      pMaskBase, JNI_ABORT);
                SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
                SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
                return;
            }

            width = srcInfo.bounds.x2 - srcInfo.bounds.x1;
            height = srcInfo.bounds.y2 - srcInfo.bounds.y1;
            maskoff += ((srcInfo.bounds.y1 - srcy) * maskscan +
                        (srcInfo.bounds.x1 - srcx));
            srcScanStride -= width * srcPixelStride;
            newscan = maskscan - width;
            pMask += maskoff;
            rgbavals = rgbamask + maskoff;
            ea = (GLubyte)(oglc->extraAlpha * 255);
            h = height;

            J2dTraceLn4(J2D_TRACE_VERBOSE, "sx=%d sy=%d w=%d h=%d",
                        srcInfo.bounds.x1, srcInfo.bounds.y1, width, height);
            J2dTraceLn2(J2D_TRACE_VERBOSE, "dx=%d dy=%d", dstx, dsty);
            J2dTraceLn3(J2D_TRACE_VERBOSE, "maskoff=%d maskscan=%d newscan=%d",
                        maskoff, maskscan, newscan);
            J2dTraceLn2(J2D_TRACE_VERBOSE, "pixstride=%d scanstride=%d",
                        srcPixelStride, srcScanStride);

            // apply alpha values from mask to the source tile, and store
            // results in an intermediate "IntRgbaPre" buffer (there are
            // separate inner loops for the three most common source formats)
            switch (srctype) {
            case sun_java2d_opengl_OGLSurfaceData_PF_INT_ARGB:
                do {
                    jint w = width;
                    do {
                        jint pathA = *pMask++;
                        if (!pathA) {
                            rgbavals[0] = 0;
                        } else {
                            jint cr, cg, cb, ca;
                            GLubyte r, g, b, a;
                            LoadIntArgbTo4ByteArgb(pSrc, c, 0, ca, cr, cg, cb);
                            a = MUL8(MUL8(ca, pathA), ea);
                            r = MUL8(cr, a);
                            g = MUL8(cg, a);
                            b = MUL8(cb, a);
                            rgbavals[0] = (r << 24) | (g << 16) | (b << 8) | a;
                        }
                        pSrc = PtrAddBytes(pSrc, srcPixelStride);
                        rgbavals++;
                    } while (--w > 0);
                    pSrc = PtrAddBytes(pSrc, srcScanStride);
                    pMask = PtrAddBytes(pMask, newscan);
                    rgbavals = PtrAddBytes(rgbavals, newscan * 4);
                } while (--h > 0);
                break;

            case sun_java2d_opengl_OGLSurfaceData_PF_INT_RGB:
                do {
                    jint w = width;
                    do {
                        jint pathA = *pMask++;
                        if (!pathA) {
                            rgbavals[0] = 0;
                        } else {
                            jint cr, cg, cb;
                            GLubyte r, g, b, a;
                            LoadIntRgbTo3ByteRgb(pSrc, c, 0, cr, cg, cb);
                            a = MUL8(pathA, ea);
                            r = MUL8(cr, a);
                            g = MUL8(cg, a);
                            b = MUL8(cb, a);
                            rgbavals[0] = (r << 24) | (g << 16) | (b << 8) | a;
                        }
                        pSrc = PtrAddBytes(pSrc, srcPixelStride);
                        rgbavals++;
                    } while (--w > 0);
                    pSrc = PtrAddBytes(pSrc, srcScanStride);
                    pMask = PtrAddBytes(pMask, newscan);
                    rgbavals = PtrAddBytes(rgbavals, newscan * 4);
                } while (--h > 0);
                break;

            case sun_java2d_opengl_OGLSurfaceData_PF_INT_BGR:
                do {
                    jint w = width;
                    do {
                        jint pathA = *pMask++;
                        if (!pathA) {
                            rgbavals[0] = 0;
                        } else {
                            jint cr, cg, cb;
                            GLubyte r, g, b, a;
                            LoadIntBgrTo3ByteRgb(pSrc, c, 0, cr, cg, cb);
                            a = MUL8(pathA, ea);
                            r = MUL8(cr, a);
                            g = MUL8(cg, a);
                            b = MUL8(cb, a);
                            rgbavals[0] = (r << 24) | (g << 16) | (b << 8) | a;
                        }
                        pSrc = PtrAddBytes(pSrc, srcPixelStride);
                        rgbavals++;
                    } while (--w > 0);
                    pSrc = PtrAddBytes(pSrc, srcScanStride);
                    pMask = PtrAddBytes(pMask, newscan);
                    rgbavals = PtrAddBytes(rgbavals, newscan * 4);
                } while (--h > 0);
                break;

            default:
                // should not get here, just no-op...
                break;
            }

            // set up texture parameters
            j2d_glEnable(GL_TEXTURE_2D);
            j2d_glBindTexture(GL_TEXTURE_2D, oglc->blitTextureID);
            j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                GL_NEAREST);
            j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                GL_NEAREST);

            j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, maskscan);
            j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, maskoff / maskscan);
            j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, maskoff % maskscan);
            j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

            // copy system memory IntRgbaPre surface into cached texture
            j2d_glTexSubImage2D(GL_TEXTURE_2D, 0,
                                0, 0, width, height,
                                GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, rgbamask);

            tx1 = 0.0f;
            ty1 = 0.0f;
            tx2 = ((GLfloat)width) / OGLSD_BLIT_TILE_SIZE;
            ty2 = ((GLfloat)height) / OGLSD_BLIT_TILE_SIZE;

            // render cached texture to the OpenGL surface
            j2d_glBegin(GL_QUADS);
            j2d_glTexCoord2f(tx1, ty1);
            j2d_glVertex2i(dstx, dsty);
            j2d_glTexCoord2f(tx2, ty1);
            j2d_glVertex2i(dstx + width, dsty);
            j2d_glTexCoord2f(tx2, ty2);
            j2d_glVertex2i(dstx + width, dsty + height);
            j2d_glTexCoord2f(tx1, ty2);
            j2d_glVertex2i(dstx, dsty + height);
            j2d_glEnd();

            j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
            j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);

            j2d_glDisable(GL_TEXTURE_2D);

            OGLContext_Flush(env, oglc);

            (*env)->ReleasePrimitiveArrayCritical(env, maskArray,
                                                  pMaskBase, JNI_ABORT);
        }
	SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
    }
    SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
}

#if 0
/**
 * REMIND:
 * This is an alternate implementation of MaskBlit that is currently
 * unused.  It relies on the GL_ARB_multitexture extension, which
 * is only available on newer framebuffers.  It does not appear to be any
 * faster than the implementation above, but I'm leaving it in here
 * for future performance comparison.
 *
 * The source color (sclr) is originally specified as 0xffffffff (all 1.0's),
 * but the extra alpha value will be incorporated into the current color in
 * the InitColor() step in GetOGLContext().
 *
 *   sclr   ea   ea   ea   ea    (the source color)
 *   stx0   a0   r0   g0   b0    (a pixel in the source sysmem tile)
 *   stx1   a1   a1   a1   a1    (an alpha value in the mask tile)
 *
 * Since GL_MODULATE is the texture application mode for both texture tiles,
 * and multitexturing is enabled, we will effectively multiply each component
 * in sclr by stx0, and then multiply stx1 to get the final fragment value.
 * Note that this approach will work for non-premultiplied surfaces only if
 * the "blendPremult" extension is available (and even then, we would need
 * to make some modifications here to avoid multiplying the mask alpha twice).
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLMaskBlit_MaskBlit
    (JNIEnv *env, jobject self,
     jobject srcData, jobject dstData,
     jobject comp, jobject clip,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height,
     jbyteArray maskArray,
     jint maskoff, jint maskscan,
     jint srctype)
{
    SurfaceDataOps *srcOps;
    OGLSDOps *dstOps;
    OGLContext *oglc;
    SurfaceDataRasInfo srcInfo, dstInfo;
    OGLPixelFormat pf = PixelFormats[srctype];

    J2dTraceLn(J2D_TRACE_INFO, "in OGLMaskBlit_MaskBlit");

    if (width <= 0 || height <= 0) {
        J2dTraceLn(J2D_TRACE_WARNING, "invalid dimensions");
        return;
    }

    if (maskArray == NULL) {
        J2dTraceLn(J2D_TRACE_WARNING, "maskArray is null");
        return;
    }

    srcOps = SurfaceData_GetOps(env, srcData);
    dstOps = OGLSurfaceData_GetOps(env, dstData);
    if (srcOps == NULL || dstOps == NULL) {
        J2dTraceLn(J2D_TRACE_WARNING, "ops are null");
        return;
    }

    srcInfo.bounds.x1 = srcx;
    srcInfo.bounds.y1 = srcy;
    srcInfo.bounds.x2 = srcx + width;
    srcInfo.bounds.y2 = srcy + height;
    dstInfo.bounds.x1 = dstx;
    dstInfo.bounds.y1 = dsty;
    dstInfo.bounds.x2 = dstx + width;
    dstInfo.bounds.y2 = dsty + height;

    if (srcOps->Lock(env, srcOps, &srcInfo, SD_LOCK_READ) != SD_SUCCESS) {
        J2dTraceLn(J2D_TRACE_WARNING, "could not acquire lock");
        return;
    }

    SurfaceData_IntersectBlitBounds(&srcInfo.bounds, &dstInfo.bounds,
                                    dstx - srcx, dsty - srcy);

    if (srcInfo.bounds.x2 > srcInfo.bounds.x1 &&
	srcInfo.bounds.y2 > srcInfo.bounds.y1)
    {
        srcOps->GetRasInfo(env, srcOps, &srcInfo);
	if (srcInfo.rasBase) {
            jsize masklen = (*env)->GetArrayLength(env, maskArray);
            unsigned char *pMask = 
                (*env)->GetPrimitiveArrayCritical(env, maskArray, 0);
            GLfloat tx1, ty1, tx2, ty2;
            jint ctxflags = pf.isPremult ? OGLSD_SRC_IS_PREMULT :
                                           OGLSD_NO_CONTEXT_FLAGS;

            if (pMask == NULL) {
                J2dTraceLn(J2D_TRACE_ERROR, "could not lock mask array");
                SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
                SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
                return;
            }

            if (masklen > MASK_LENGTH) {
                // REMIND: this approach is seriously flawed if the mask
                //         length is ever greater than MASK_LENGTH (won't fit
                //         into the cached mask tile)...
                J2dTraceLn(J2D_TRACE_ERROR, "mask array too large (FIXME)");
                (*env)->ReleasePrimitiveArrayCritical(env, maskArray,
                                                      pMask, JNI_ABORT);
                SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
                SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
                return;
            }

            oglc = dstOps->GetOGLContext(env, dstOps, dstOps,
                                         clip, comp, NULL, 0xffffffff,
                                         ctxflags);
            if (oglc == NULL) {
                J2dTraceLn(J2D_TRACE_ERROR, "could not get OGL context");
                (*env)->ReleasePrimitiveArrayCritical(env, maskArray,
                                                      pMask, JNI_ABORT);
                SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
                SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
                return;
            }

            if (oglc->maskTextureID == 0) {
                if (OGLSD_InitMaskTileTexture(oglc) == SD_FAILURE) {
                    J2dTraceLn(J2D_TRACE_ERROR, "could not init mask tile");
                    (*env)->ReleasePrimitiveArrayCritical(env, maskArray,
                                                          pMask, JNI_ABORT);
                    dstOps->ReleaseOGLContext(env, dstOps, oglc);
                    SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
                    SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
                    return;
                }
            }

            if (oglc->blitTextureID == 0) {
                if (OGLSD_InitBlitTileTexture(oglc) == SD_FAILURE) {
                    J2dTraceLn(J2D_TRACE_ERROR, "could not init blit tile");
                    (*env)->ReleasePrimitiveArrayCritical(env, maskArray,
                                                          pMask, JNI_ABORT);
                    dstOps->ReleaseOGLContext(env, dstOps, oglc);
                    SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
                    SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
                    return;
                }
            }

            width = srcInfo.bounds.x2 - srcInfo.bounds.x1;
            height = srcInfo.bounds.y2 - srcInfo.bounds.y1;
            maskoff += ((srcInfo.bounds.y1 - srcy) * maskscan +
                        (srcInfo.bounds.x1 - srcx));

            J2dTraceLn4(J2D_TRACE_VERBOSE, "w=%d h=%d maskoff=%d maskscan=%d",
                        width, height, maskoff, maskscan);

            // transfer src pixel data into cached RGBA texture
            if (!pf.hasAlpha) {
                j2d_glPixelTransferf(GL_ALPHA_BIAS, 1.0f);
            }
            j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, srcInfo.bounds.x1);
            j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, srcInfo.bounds.y1);
            j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH,
                              srcInfo.scanStride / srcInfo.pixelStride);
            j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            j2d_glBindTexture(GL_TEXTURE_2D, oglc->blitTextureID);
            j2d_glTexSubImage2D(GL_TEXTURE_2D, 0,
                                0, 0, width, height,
                                pf.format, pf.type, srcInfo.rasBase);

            j2d_glPixelTransferf(GL_ALPHA_BIAS, 0.0f);

            // transfer mask data into cached INTENSITY texture
            j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, maskoff % maskscan);
            j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, maskoff / maskscan);
            j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, maskscan);

            j2d_glBindTexture(GL_TEXTURE_2D, oglc->maskTextureID);
            j2d_glTexSubImage2D(GL_TEXTURE_2D, 0,
                                0, 0, width, height,
                                GL_LUMINANCE, GL_UNSIGNED_BYTE, pMask);

            j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
            j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

            j2d_glActiveTextureARB(GL_TEXTURE0);
            j2d_glEnable(GL_TEXTURE_2D);
            j2d_glBindTexture(GL_TEXTURE_2D, oglc->blitTextureID);
            j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            j2d_glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            j2d_glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            j2d_glActiveTextureARB(GL_TEXTURE1);
            j2d_glEnable(GL_TEXTURE_2D);
            j2d_glBindTexture(GL_TEXTURE_2D, oglc->maskTextureID);
            j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            j2d_glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            j2d_glTexParameteri(GL_TEXTURE_2D,
                                GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            // REMIND: this assumes MASK_TILE_SIZE == BLIT_TILE_SIZE
            tx1 = 0.0f;
            ty1 = 0.0f;
            tx2 = (((GLfloat)width) / OGLSD_MASK_TILE_SIZE);
            ty2 = (((GLfloat)height) / OGLSD_MASK_TILE_SIZE);

            j2d_glBegin(GL_QUADS);
            j2d_glMultiTexCoord2fARB(GL_TEXTURE0, tx1, ty1);
            j2d_glMultiTexCoord2fARB(GL_TEXTURE1, tx1, ty1);
            j2d_glVertex2i(dstInfo.bounds.x1, dstInfo.bounds.y1);
            j2d_glMultiTexCoord2fARB(GL_TEXTURE0, tx2, ty1);
            j2d_glMultiTexCoord2fARB(GL_TEXTURE1, tx2, ty1);
            j2d_glVertex2i(dstInfo.bounds.x2, dstInfo.bounds.y1);
            j2d_glMultiTexCoord2fARB(GL_TEXTURE0, tx2, ty2);
            j2d_glMultiTexCoord2fARB(GL_TEXTURE1, tx2, ty2);
            j2d_glVertex2i(dstInfo.bounds.x2, dstInfo.bounds.y2);
            j2d_glMultiTexCoord2fARB(GL_TEXTURE0, tx1, ty2);
            j2d_glMultiTexCoord2fARB(GL_TEXTURE1, tx1, ty2);
            j2d_glVertex2i(dstInfo.bounds.x1, dstInfo.bounds.y2);
            j2d_glEnd();

            j2d_glActiveTextureARB(GL_TEXTURE1);
            j2d_glDisable(GL_TEXTURE_2D);
            j2d_glActiveTextureARB(GL_TEXTURE0);
            j2d_glDisable(GL_TEXTURE_2D);

            (*env)->ReleasePrimitiveArrayCritical(env, maskArray,
                                                  pMask, JNI_ABORT);

            dstOps->ReleaseOGLContext(env, dstOps, oglc);
        }
	SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
    }
    SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
}
#endif /* 0 */

#endif /* !HEADLESS */
