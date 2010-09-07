/*
 * @(#)OGLBlitLoops.c	1.10 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef HEADLESS

#include <jni.h>
#include <jlong.h>

#include "sun_java2d_opengl_OGLBlitLoops.h"

#include "SurfaceData.h"
#include "OGLBlitLoops.h"
#include "OGLSurfaceData.h"
#include "GraphicsPrimitiveMgr.h"

extern OGLPixelFormat PixelFormats[];

#define OGLSD_COLOR_MAP_SIZE 256
static GLushort colorMap[OGLSD_COLOR_MAP_SIZE];
static jboolean colorMapInited = JNI_FALSE;

/**
 * Initializes the global color map used in blit operations involving indexed
 * system memory surfaces.
 *
 * REMIND: check maximum table size (can be implementation dependent)...
 */
static void
OGLSD_InitColorMap() {
    int i;
    
    for (i = 0; i < OGLSD_COLOR_MAP_SIZE; i++) {
        colorMap[i] = (GLushort)((i << 8) | i);
    }

    colorMapInited = JNI_TRUE;
}

/**
 * Inner loop used for copying a source OpenGL "Surface" (window, pbuffer,
 * etc.) to a destination OpenGL "Surface".  Note that the same surface can
 * be used as both the source and destination, as is the case in a copyArea()
 * operation.  This method is invoked from OGLBlitLoops_IsoBlit() as well as
 * OGLRenderer_devCopyArea().
 *
 * The standard glCopyPixels() mechanism is used to copy the source region
 * into the destination region.  If the regions have different dimensions,
 * the source will be scaled into the destination as appropriate (only
 * nearest neighbor filtering will be applied for simple scale operations).
 */
void
OGLBlitSurfaceToSurface(OGLSDOps *srcOps, OGLSDOps *dstOps,
                        jint srcx, jint srcy, jint dstx, jint dsty,
                        jint srcw, jint srch, jint dstw, jint dsth)
{
    GLfloat scalex, scaley;

    scalex = ((GLfloat)dstw) / srcw;
    scaley = ((GLfloat)dsth) / srch;

    // the following lines account for the fact that glCopyPixels() copies a
    // region whose lower-left corner is at (x,y), but the source parameters
    // (srcx,srcy) we're given here point to the upper-left corner of the
    // source region... so here we play with the srcy and dsty parameters so
    // that they point to the lower-left corners of the regions... 
    srcx = srcOps->xOffset + srcx;
    srcy = srcOps->yOffset + srcOps->height - (srcy + srch);
    dsty += dsth;
    if (dsty > dstOps->height) {
        jint ydiff = (jint)(((GLfloat)(dsty - dstOps->height)) / scaley);
        srcy += ydiff;
        dsty = dstOps->height;
    }

    // see OGLBlitSwToSurface() for more info on the following two lines
    j2d_glRasterPos2i(0, 0);
    j2d_glBitmap(0, 0, 0, 0, (GLfloat)dstx, (GLfloat)-dsty, NULL);

    j2d_glPixelZoom(scalex, scaley);
    j2d_glCopyPixels(srcx, srcy, srcw, srch, GL_COLOR);
    j2d_glPixelZoom(1.0, 1.0);
}

/**
 * Inner loop used for copying a source OpenGL "Texture" to a destination
 * OpenGL "Surface".  This method is invoked from OGLBlitLoops_IsoBlit().
 *
 * This method will copy, scale, or transform the source texture into the
 * destination depending on the transform state, as established in
 * and OGLContext_setTransform().  If the source texture is
 * transformed in any way when rendered into the destination, the filtering
 * method applied is determined by the hint parameter (can be GL_NEAREST or
 * GL_LINEAR).
 */
static void
OGLBlitTextureToSurface(OGLSDOps *srcOps, OGLSDOps *dstOps,
                        jboolean rtt, jint hint,
                        jint srcx, jint srcy, jint dstx, jint dsty,
                        jint srcw, jint srch, jint dstw, jint dsth)
{
    GLfloat tx1, ty1, tx2, ty2;
    GLfloat y1, y2;

    tx1 = ((GLfloat)srcx) / srcOps->textureWidth;
    ty1 = ((GLfloat)srcy) / srcOps->textureHeight;
    tx2 = tx1 + (((GLfloat)srcw) / srcOps->textureWidth);
    ty2 = ty1 + (((GLfloat)srch) / srcOps->textureHeight);

    // if the source is a render-to-texture surface, we must flip the
    // texture coordinates vertically
    y1 = rtt ? ty2 : ty1;
    y2 = rtt ? ty1 : ty2;

    j2d_glEnable(GL_TEXTURE_2D);
    j2d_glBindTexture(GL_TEXTURE_2D, srcOps->textureID);
    j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, hint);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, hint);

    j2d_glBegin(GL_QUADS);
    j2d_glTexCoord2f(tx1, y1); j2d_glVertex2i(dstx, dsty);
    j2d_glTexCoord2f(tx2, y1); j2d_glVertex2i(dstx + dstw, dsty);
    j2d_glTexCoord2f(tx2, y2); j2d_glVertex2i(dstx + dstw, dsty + dsth);
    j2d_glTexCoord2f(tx1, y2); j2d_glVertex2i(dstx, dsty + dsth);
    j2d_glEnd();

    j2d_glDisable(GL_TEXTURE_2D);
}

/**
 * Inner loop used for copying a source system memory ("Sw") surface to a
 * destination OpenGL "Surface".  This method is invoked from
 * OGLBlitLoops_Blit().
 *
 * The standard glDrawPixels() mechanism is used to copy the source region
 * into the destination region.  If the regions have different
 * dimensions, the source will be scaled into the destination
 * as appropriate (only nearest neighbor filtering will be applied for simple
 * scale operations).
 */
static void
OGLBlitSwToSurface(SurfaceDataRasInfo *srcInfo,
                   OGLPixelFormat *pf,
                   jint srcx, jint srcy, jint dstx, jint dsty,
                   jint srcw, jint srch, jint dstw, jint dsth)
{
    GLfloat scalex, scaley;

    scalex = ((GLfloat)dstw) / srcw;
    scaley = ((GLfloat)dsth) / srch;

    // This is a rather intriguing (yet totally valid) hack... If we were to
    // specify a raster position that is outside the surface bounds, the raster
    // position would be invalid and nothing would be rendered.  However, we
    // can use a widely known trick to move the raster position outside the
    // surface bounds while maintaining its status as valid.  The following
    // call to glBitmap() renders a no-op bitmap, but offsets the current
    // raster position from (0,0) to the desired location of (dstx,-dsty)...
    j2d_glRasterPos2i(0, 0);
    j2d_glBitmap(0, 0, 0, 0, (GLfloat)dstx, (GLfloat)-dsty, NULL);

    j2d_glPixelZoom(scalex, -scaley);
    j2d_glDrawPixels(srcw, srch, pf->format, pf->type, srcInfo->rasBase);
    j2d_glPixelZoom(1.0, 1.0);
}

/**
 * Inner loop used for copying a source system memory ("Sw") surface or
 * OpenGL "Surface" to a destination OpenGL "Surface", using an OpenGL texture
 * tile as an intermediate surface.  This method is invoked from
 * OGLBlitLoops_Blit() for "Sw" surfaces and OGLBlitLoops_IsoBlit() for
 * "Surface" surfaces.
 *
 * This method is used to transform the source surface into the destination.
 * Pixel rectangles cannot be arbitrarily transformed (without the
 * GL_EXT_pixel_transform extension, which is not supported on most modern
 * hardware).  However, texture mapped quads do respect the GL_MODELVIEW
 * transform matrix, so we use textures here to perform the transform
 * operation.  This method uses a tile-based approach in which a small
 * subregion of the source surface is copied into a cached texture tile.  The
 * texture tile is then mapped into the appropriate location in the
 * destination surface.
 *
 * REMIND: this only works well using GL_NEAREST for the filtering mode
 *         (GL_LINEAR causes visible stitching problems between tiles,
 *         but this can be fixed by making use of texture borders)
 */
static void
OGLBlitToSurfaceViaTexture(OGLContext *oglc, SurfaceDataRasInfo *srcInfo,
                           OGLPixelFormat *pf, OGLSDOps *srcOps,
                           jboolean swsurface, jint hint,
                           jint srcx, jint srcy, jint dstx, jint dsty,
                           jint srcw, jint srch, jint dstw, jint dsth)
{
    GLfloat tx1, ty1, tx2, ty2;
    GLfloat dx1, dy1, dx2, dy2;
    GLfloat dx, dy, dw, dh, cdw, cdh;
    jint tw, th;
    jint sx1, sy1, sx2, sy2;
    jint sx, sy, sw, sh;
    GLint glhint = (hint == OGLSD_XFORM_BILINEAR) ? GL_LINEAR : GL_NEAREST;

    if (oglc->blitTextureID == 0) {
        if (OGLSD_InitBlitTileTexture(oglc) == SD_FAILURE) {
            J2dTraceLn(J2D_TRACE_ERROR, "could not init blit tile");
            return;
        }
    }

    // REMIND: why do we need so many variables?!?!
    tx1 = 0.0f;
    ty1 = 0.0f;
    tw = OGLSD_BLIT_TILE_SIZE;
    th = OGLSD_BLIT_TILE_SIZE;
    sx1 = srcx;
    sy1 = srcy;
    sx2 = srcx + srcw;
    sy2 = srcy + srch;
    dx1 = (GLfloat)dstx;
    dy1 = (GLfloat)dsty;
    dx2 = (GLfloat)(dstx + dstw);
    dy2 = (GLfloat)(dsty + dsth);
    cdw = ((GLfloat)dstw) / (((GLfloat)srcw) / OGLSD_BLIT_TILE_SIZE);
    cdh = ((GLfloat)dsth) / (((GLfloat)srch) / OGLSD_BLIT_TILE_SIZE);

    j2d_glEnable(GL_TEXTURE_2D);
    j2d_glBindTexture(GL_TEXTURE_2D, oglc->blitTextureID);
    j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glhint);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glhint);

    for (sy = sy1, dy = dy1; sy < sy2; sy += th, dy += cdh) {
        sh = ((sy + th) > sy2) ? (sy2 - sy) : th;
        dh = ((dy + cdh) > dy2) ? (dy2 - dy) : cdh;

        for (sx = sx1, dx = dx1; sx < sx2; sx += tw, dx += cdw) {
            sw = ((sx + tw) > sx2) ? (sx2 - sx) : tw;
            dw = ((dx + cdw) > dx2) ? (dx2 - dx) : cdw;

            tx2 = ((GLfloat)sw) / tw;
            ty2 = ((GLfloat)sh) / th;

            if (swsurface) {
                j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, sx);
                j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, sy);

                j2d_glTexSubImage2D(GL_TEXTURE_2D, 0,
                                    0, 0, sw, sh,
                                    pf->format, pf->type,
                                    srcInfo->rasBase);

                // the texture image is "right side up", so we align the
                // upper-left texture corner with the upper-left quad corner
                j2d_glBegin(GL_QUADS);
                j2d_glTexCoord2f(tx1, ty1); j2d_glVertex2f(dx, dy);
                j2d_glTexCoord2f(tx2, ty1); j2d_glVertex2f(dx + dw, dy);
                j2d_glTexCoord2f(tx2, ty2); j2d_glVertex2f(dx + dw, dy + dh);
                j2d_glTexCoord2f(tx1, ty2); j2d_glVertex2f(dx, dy + dh);
                j2d_glEnd();
            } else {
                // this accounts for lower-left origin of the source region
                jint newsx = srcOps->xOffset + sx;
                jint newsy = srcOps->yOffset + srcOps->height - (sy + sh);
                j2d_glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
                                        0, 0, newsx, newsy, sw, sh);

                // the texture image is "upside down" after the last step, so
                // we align the bottom-left texture corner with the upper-left
                // quad corner (and vice versa) to effectively flip the
                // texture image
                j2d_glBegin(GL_QUADS);
                j2d_glTexCoord2f(tx1, ty2); j2d_glVertex2f(dx, dy);
                j2d_glTexCoord2f(tx2, ty2); j2d_glVertex2f(dx + dw, dy);
                j2d_glTexCoord2f(tx2, ty1); j2d_glVertex2f(dx + dw, dy + dh);
                j2d_glTexCoord2f(tx1, ty1); j2d_glVertex2f(dx, dy + dh);
                j2d_glEnd();
            }
        }
    }

    j2d_glDisable(GL_TEXTURE_2D);
}

/**
 * Inner loop used for copying a source system memory ("Sw") surface to a
 * destination OpenGL "Texture".  This method is invoked from
 * OGLBlitLoops_Blit().
 *
 * The source surface is effectively loaded into the OpenGL texture object,
 * which must have already been initialized by OGLSD_initTexture().  Note
 * that this method is only capable of copying the source surface into the
 * destination surface (i.e. no scaling or general transform is allowed).
 * This restriction should not be an issue as this method is only used
 * currently to cache a static system memory image into an OpenGL texture in
 * a hidden-acceleration situation.
 */
static void
OGLBlitSwToTexture(SurfaceDataRasInfo *srcInfo, OGLPixelFormat *pf,
                   OGLSDOps *dstOps,
                   jint dstx, jint dsty, jint width, jint height)
{
    j2d_glEnable(GL_TEXTURE_2D);
    j2d_glBindTexture(GL_TEXTURE_2D, dstOps->textureID);
    j2d_glTexSubImage2D(GL_TEXTURE_2D, 0,
                        dstx, dsty, width, height,
                        pf->format, pf->type, srcInfo->rasBase);
    j2d_glDisable(GL_TEXTURE_2D);

    // REMIND: we copy the premult status of the Sw surface into the Texture
    //         ops so that we can decide later (when doing Texture->Surface
    //         copies) whether we need to multiply the alpha on the fly... 
    dstOps->isPremult = pf->isPremult;
}

/**
 * General blit method for copying a native OpenGL surface (of type "Surface"
 * or "Texture") to another OpenGL "Surface".  If texture is JNI_TRUE, this
 * method will invoke the Texture->Surface inner loop; otherwise, one of the
 * Surface->Surface inner loops will be invoked, depending on the transform
 * state.
 *
 * REMIND: we can trick these blit methods into doing XOR simply by passing
 *         in the (pixel ^ xorpixel) as the pixel value and preceding the
 *         blit with a fillrect...
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLBlitLoops_IsoBlit
    (JNIEnv *env, jclass oglblit,
     jlong pCtx, jlong pSrcOps, jlong pDstOps,
     jobject xform, jint hint,
     jint srcx, jint srcy, jint dstx, jint dsty,
     jint srcw, jint srch, jint dstw, jint dsth,
     jboolean texture, jboolean rtt)
{
    OGLSDOps *srcOps = (OGLSDOps *)jlong_to_ptr(pSrcOps);
    OGLSDOps *dstOps = (OGLSDOps *)jlong_to_ptr(pDstOps);
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    SurfaceDataRasInfo srcInfo;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLIsoBlit");

    if (srcw <= 0 || srch <= 0 || dstw <= 0 || dsth <= 0) {
        J2dTraceLn(J2D_TRACE_WARNING, "invalid dimensions or srctype");
        return;
    }

    if (srcOps == NULL || dstOps == NULL) {
        J2dTraceLn(J2D_TRACE_WARNING, "ops are null");
	return;
    }

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "context is null");
	return;
    }

    srcInfo.bounds.x1 = srcx;
    srcInfo.bounds.y1 = srcy;
    srcInfo.bounds.x2 = srcx + srcw;
    srcInfo.bounds.y2 = srcy + srch;

    SurfaceData_IntersectBoundsXYXY(&srcInfo.bounds,
                                    0, 0, srcOps->width, srcOps->height);

    if (srcInfo.bounds.x2 > srcInfo.bounds.x1 &&
	srcInfo.bounds.y2 > srcInfo.bounds.y1)
    {
        srcx = srcInfo.bounds.x1;
        srcy = srcInfo.bounds.y1;
        srcw = srcInfo.bounds.x2 - srcInfo.bounds.x1;
        srch = srcInfo.bounds.y2 - srcInfo.bounds.y1;

        J2dTraceLn2(J2D_TRACE_VERBOSE, "texture=%d hint=%d", texture, hint);
        J2dTraceLn4(J2D_TRACE_VERBOSE, "sx=%d sy=%d sw=%d sh=%d",
                    srcx, srcy, srcw, srch);
        J2dTraceLn4(J2D_TRACE_VERBOSE, "dx=%d dy=%d dw=%d dh=%d",
                    dstx, dsty, dstw, dsth);

        if (texture) {
            GLint glhint = (hint == OGLSD_XFORM_BILINEAR) ? GL_LINEAR :
                                                            GL_NEAREST;
            OGLBlitTextureToSurface(srcOps, dstOps, rtt, glhint,
                                    srcx, srcy, dstx, dsty,
                                    srcw, srch, dstw, dsth);
        } else {
            if (xform == NULL) {
                OGLBlitSurfaceToSurface(srcOps, dstOps,
                                        srcx, srcy, dstx, dsty,
                                        srcw, srch, dstw, dsth);
            } else {
                OGLBlitToSurfaceViaTexture(oglc, &srcInfo, NULL, srcOps,
                                           JNI_FALSE, hint,
                                           srcx, srcy, dstx, dsty,
                                           srcw, srch, dstw, dsth);
            }
        }
    }

    OGLContext_Flush(env, oglc);
}

/**
 * General blit method for copying a system memory ("Sw") surface to a native
 * OpenGL surface (of type "Surface" or "Texture").  If texture is JNI_TRUE,
 * this method will invoke the Sw->Texture inner loop; otherwise, one of the
 * Sw->Surface inner loops will be invoked, depending on the transform state.
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLBlitLoops_Blit
    (JNIEnv *env, jclass oglblit,
     jlong pCtx, jlong pSrcOps, jlong pDstOps,
     jobject xform, jint hint,
     jint srcx, jint srcy, jint dstx, jint dsty,
     jint srcw, jint srch, jint dstw, jint dsth,
     jint srctype, jboolean texture)
{
    SurfaceDataOps *srcOps = (SurfaceDataOps *)jlong_to_ptr(pSrcOps);
    OGLSDOps *dstOps = (OGLSDOps *)jlong_to_ptr(pDstOps);
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    SurfaceDataRasInfo srcInfo;
    OGLPixelFormat pf = PixelFormats[srctype];
    jint lockflags = SD_LOCK_READ | pf.lockFlags;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLBlit");

    if (srcw <= 0 || srch <= 0 || dstw <= 0 || dsth <= 0 || srctype < 0) {
        J2dTraceLn(J2D_TRACE_WARNING, "invalid dimensions or srctype");
        return;
    }

    if (srcOps == NULL || dstOps == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
	return;
    }

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "context is null");
	return;
    }

    srcInfo.bounds.x1 = srcx;
    srcInfo.bounds.y1 = srcy;
    srcInfo.bounds.x2 = srcx + srcw;
    srcInfo.bounds.y2 = srcy + srch;

    if (srcOps->Lock(env, srcOps, &srcInfo, lockflags) != SD_SUCCESS) {
        J2dTraceLn(J2D_TRACE_WARNING, "could not acquire lock");
	return;
    }

    if (srcInfo.bounds.x2 > srcInfo.bounds.x1 &&
	srcInfo.bounds.y2 > srcInfo.bounds.y1)
    {
        srcOps->GetRasInfo(env, srcOps, &srcInfo);
	if (srcInfo.rasBase) {
            srcx = srcInfo.bounds.x1;
            srcy = srcInfo.bounds.y1;
            srcw = srcInfo.bounds.x2 - srcInfo.bounds.x1;
            srch = srcInfo.bounds.y2 - srcInfo.bounds.y1;

            J2dTraceLn3(J2D_TRACE_VERBOSE, "texture=%d srctype=%d hint=%d",
                        texture, srctype, hint);
            J2dTraceLn4(J2D_TRACE_VERBOSE, "sx=%d sy=%d sw=%d sh=%d",
                        srcx, srcy, srcw, srch);
            J2dTraceLn4(J2D_TRACE_VERBOSE, "dx=%d dy=%d dw=%d dh=%d",
                        dstx, dsty, dstw, dsth);

            if (srcInfo.lutBase) {
                // REMIND: size must be power of two
                GLint size = srcInfo.lutSize;

                if (!colorMapInited) {
                    OGLSD_InitColorMap();
                }

                j2d_glPixelMapusv(GL_PIXEL_MAP_I_TO_A, size, colorMap);
                j2d_glPixelMapusv(GL_PIXEL_MAP_I_TO_R, size, colorMap);
                j2d_glPixelMapusv(GL_PIXEL_MAP_I_TO_G, size, colorMap);
                j2d_glPixelMapusv(GL_PIXEL_MAP_I_TO_B, size, colorMap);

                j2d_glEnable(GL_COLOR_TABLE);
                j2d_glColorTable(GL_COLOR_TABLE, GL_RGBA, size,
                                 GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                                 srcInfo.lutBase);
            }

            if (!pf.hasAlpha) {
                // if the source surface does not have an alpha channel,
                // we need to ensure that the alpha values are forced to
                // the current extra alpha value (see OGLSD_InitExtraAlpha
                // for more information)
                j2d_glPixelTransferf(GL_ALPHA_SCALE, 0.0f);
                j2d_glPixelTransferf(GL_ALPHA_BIAS, oglc->extraAlpha);
            }
            j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, srcx);
            j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, srcy);
            j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH,
                              srcInfo.scanStride / srcInfo.pixelStride);
            j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, pf.alignment);

            if (texture) {
                OGLBlitSwToTexture(&srcInfo, &pf, dstOps,
                                   dstx, dsty, dstw, dsth);
            } else {
                if (xform == NULL) {
                    OGLBlitSwToSurface(&srcInfo, &pf,
                                       srcx, srcy, dstx, dsty,
                                       srcw, srch, dstw, dsth);
                } else {
                    OGLBlitToSurfaceViaTexture(oglc, &srcInfo, &pf, NULL,
                                               JNI_TRUE, hint,
                                               srcx, srcy, dstx, dsty,
                                               srcw, srch, dstw, dsth);
                }
            }

            if (srcInfo.lutBase) {
                j2d_glDisable(GL_COLOR_TABLE);
                // REMIND: restore default pixel mappings?
            }

            if (!pf.hasAlpha) {
                // restore scale/bias to their original values
                j2d_glPixelTransferf(GL_ALPHA_SCALE, oglc->extraAlpha);
                j2d_glPixelTransferf(GL_ALPHA_BIAS, 0.0f);
            }
            j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
            j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

            OGLContext_Flush(env, oglc);
	}
	SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
    }
    SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
}

/**
 * Specialized blit method for copying a native OpenGL "Surface" (pbuffer,
 * window, etc.) to a system memory ("Sw") surface.
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLBlitLoops_SurfaceToSwBlit
    (JNIEnv *env, jclass oglblit,
     jlong pCtx, jlong pSrcOps, jlong pDstOps,
     jint srcx, jint srcy, jint dstx, jint dsty,
     jint width, jint height, jint dsttype)
{
    OGLSDOps *srcOps = (OGLSDOps *)jlong_to_ptr(pSrcOps);
    SurfaceDataOps *dstOps = (SurfaceDataOps *)jlong_to_ptr(pDstOps);
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    SurfaceDataRasInfo srcInfo, dstInfo;
    OGLPixelFormat pf = PixelFormats[dsttype];

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSurfaceToSwBlit_Blit");

    if (width <= 0 || height <= 0) {
        J2dTraceLn(J2D_TRACE_WARNING, "dimensions are non-positive");
        return;
    }

    if (srcOps == NULL || dstOps == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
	return;
    }

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "context is null");
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

    if (dstOps->Lock(env, dstOps, &dstInfo, SD_LOCK_WRITE) != SD_SUCCESS) {
        J2dTraceLn(J2D_TRACE_WARNING, "could not acquire dst lock");
	return;
    }

    SurfaceData_IntersectBoundsXYXY(&srcInfo.bounds,
                                    0, 0, srcOps->width, srcOps->height);
    SurfaceData_IntersectBlitBounds(&dstInfo.bounds, &srcInfo.bounds,
                                    srcx - dstx, srcy - dsty);
    
    if (srcInfo.bounds.x2 > srcInfo.bounds.x1 &&
	srcInfo.bounds.y2 > srcInfo.bounds.y1)
    {
        dstOps->GetRasInfo(env, dstOps, &dstInfo);
	if (dstInfo.rasBase) {
            void *pDst = dstInfo.rasBase;

            srcx = srcInfo.bounds.x1;
            srcy = srcInfo.bounds.y1;
            dstx = dstInfo.bounds.x1;
            dsty = dstInfo.bounds.y1;
            width = srcInfo.bounds.x2 - srcInfo.bounds.x1;
            height = srcInfo.bounds.y2 - srcInfo.bounds.y1;

            j2d_glPixelStorei(GL_PACK_SKIP_PIXELS, dstx);
            j2d_glPixelStorei(GL_PACK_ROW_LENGTH,
                              dstInfo.scanStride / dstInfo.pixelStride);
            j2d_glPixelStorei(GL_PACK_ALIGNMENT, pf.alignment);

            J2dTraceLn4(J2D_TRACE_VERBOSE, "sx=%d sy=%d w=%d h=%d",
                        srcx, srcy, width, height);
            J2dTraceLn2(J2D_TRACE_VERBOSE, "dx=%d dy=%d",
                        dstx, dsty);

            // this accounts for lower-left origin of the source region
            srcx = srcOps->xOffset + srcx;
            srcy = srcOps->yOffset + srcOps->height - (srcy + 1);

            // we must read one scanline at a time because there is no way
            // to read starting at the top-left corner of the source region
            while (height > 0) {
                j2d_glPixelStorei(GL_PACK_SKIP_ROWS, dsty);
                j2d_glReadPixels(srcx, srcy, width, 1,
                                 pf.format, pf.type, pDst);
                srcy--;
                dsty++;
                height--;
            }

            j2d_glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
            j2d_glPixelStorei(GL_PACK_SKIP_ROWS, 0);
            j2d_glPixelStorei(GL_PACK_ROW_LENGTH, 0);
            j2d_glPixelStorei(GL_PACK_ALIGNMENT, 4);

            OGLContext_Flush(env, oglc);
	}
	SurfaceData_InvokeRelease(env, dstOps, &dstInfo);
    }
    SurfaceData_InvokeUnlock(env, dstOps, &dstInfo);
}

#endif /* !HEADLESS */
