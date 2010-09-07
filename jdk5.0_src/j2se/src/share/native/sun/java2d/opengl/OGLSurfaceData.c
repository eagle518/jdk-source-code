/*
 * @(#)OGLSurfaceData.c	1.15 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef HEADLESS

#include <stdlib.h>

#include "sun_java2d_opengl_OGLSurfaceData.h"

#include "jlong.h"
#include "jni_util.h"
#include "OGLSurfaceData.h"

extern void OGLSD_LockImpl(JNIEnv *env);
extern void OGLSD_UnlockImpl(JNIEnv *env, jint flushFlag);
extern jint OGLSD_DisposeOGLSurface(JNIEnv *env, OGLSDOps *oglsdo);
extern OGLContext *OGLSD_GetSharedContext(JNIEnv *env);

/**
 * This table contains the "pixel formats" for all system memory surfaces
 * that OpenGL is capable of handling, indexed by the "PF_" constants defined
 * in OGLSurfaceData.java.  These pixel formats contain information that is
 * passed to OpenGL when copying from a system memory ("Sw") surface to
 * an OpenGL "Surface" (via glDrawPixels()) or "Texture" (via glTexImage2D()).
 *
 * REMIND: Index12Gray may not work because of OpenGL's restriction on
 *         2^n length color tables
 * REMIND: 3/4Byte* surfaces won't work correctly in all cases due to byte
 *         alignment issues; we'll need to find a workaround (not trivial)
 *         to make these surfaces work, but in the meantime loops involving
 *         these surfaces have been commented out in OGLBlitLoops.java...
 */
OGLPixelFormat PixelFormats[] = {
    { GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
      4, 1, 0, 0                                     }, /* 0 - IntArgb      */
    { GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
      4, 0, 1, 0                                     }, /* 1 - IntRgb       */
    { GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
      4, 0, 1, 0                                     }, /* 2 - IntRgbx      */
    { GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV,
      4, 0, 1, 0                                     }, /* 3 - IntBgr       */
    { GL_BGRA, GL_UNSIGNED_INT_8_8_8_8,
      4, 0, 1, 0                                     }, /* 4 - IntBgrx      */
    { GL_BGR,  GL_UNSIGNED_BYTE,
      1, 0, 1, 0                                     }, /* 5 - ThreeByteBgr */
    { GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV,
      4, 1, 0, 0                                     }, /* 6 - FourByteAbgr */
    { GL_RGB,  GL_UNSIGNED_SHORT_5_6_5,
      2, 0, 1, 0                                     }, /* 7 - Ushort565Rgb */
    { GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV,
      2, 0, 1, 0                                     }, /* 8 - Ushort555Rgb */
    { GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
      2, 0, 1, 0                                     }, /* 9 - Ushort555Rgbx*/
    { GL_LUMINANCE, GL_UNSIGNED_BYTE,
      1, 0, 1, 0                                     }, /*10 - ByteGray     */
    { GL_LUMINANCE, GL_UNSIGNED_SHORT,
      2, 0, 1, 0                                     }, /*11 - UshortGray   */
    { GL_COLOR_INDEX, GL_UNSIGNED_BYTE,
      1, 1, 0, SD_LOCK_LUT                           }, /*12 - ByteIndexed  */
    { GL_COLOR_INDEX, GL_UNSIGNED_BYTE,
      1, 0, 1, SD_LOCK_LUT                           }, /*13 - Index8Gray   */
    { GL_COLOR_INDEX, GL_UNSIGNED_SHORT,
      1, 0, 1, SD_LOCK_LUT                           }, /*14 - Index12Gray  */
    { GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
      4, 1, 1, 0                                     }, /*15 - IntArgbPre   */
    { GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV,
      4, 1, 1, 0                                     }, /*16 - FrByteAbgrPre*/
};

/**
 * Given a starting value and a maximum limit, returns the first power-of-two
 * greater than the starting value.  If the resulting value is greater than
 * the maximum limit, zero is returned.
 */
jint
OGLSD_NextPowerOfTwo(jint val, jint max)
{
    jint i;

    if (val > max) {
        return 0;
    }

    for (i = 1; i < val; i *= 2);

    return i;
}

/**
 * Initializes an OpenGL texture of the given width and height.  If the
 * GL_ARB_texture_non_power_of_two extension is not present, the actual
 * texture must have power-of-two dimensions, which can result in inefficient
 * use of texture memory.  In either case, once the texture has been
 * initialized, the image stored in that texture can have
 * non-power-of-two dimensions.
 */
JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_OGLSurfaceData_initTexture
    (JNIEnv *env, jobject oglsd,
     jlong pCtx, jlong pData,
     jint width, jint height)
{
    OGLSDOps *oglsdo = (OGLSDOps *)jlong_to_ptr(pData);
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    GLuint texID;
    GLsizei texWidth, texHeight, realWidth, realHeight;
    GLint texMax;

    J2dTraceLn2(J2D_TRACE_INFO, "in OGLSurfaceData_initTexture (w=%d h=%d)",
                width, height);

    if (oglsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
        return JNI_FALSE;
    }

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "shared context is null");
        return JNI_FALSE;
    }

    j2d_glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texMax);

    if (oglc->extInfo->texNonPow2) {
        // use non-power-of-two dimensions directly
        texWidth = (width <= texMax) ? width : 0;
        texHeight = (height <= texMax) ? height : 0;
    } else {
        // find the appropriate power-of-two dimensions
        texWidth = OGLSD_NextPowerOfTwo(width, texMax);
        texHeight = OGLSD_NextPowerOfTwo(height, texMax);
    }

    J2dTraceLn3(J2D_TRACE_VERBOSE,
                "actual texture dimensions (w=%d h=%d max=%d)",
                texWidth, texHeight, texMax);

    // if either dimension is 0, we cannot allocate a texture with the
    // requested dimensions
    if ((texWidth == 0) || (texHeight == 0)) {
        J2dTraceLn(J2D_TRACE_ERROR, "texture dimensions too large");
        return JNI_FALSE;
    }

    // now use a proxy to determine whether we can create a texture with
    // the calculated power-of-two dimensions and the given internal format
    j2d_glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA,
                     texWidth, texHeight, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    j2d_glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0,
                                 GL_TEXTURE_WIDTH, &realWidth);
    j2d_glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0,
                                 GL_TEXTURE_HEIGHT, &realHeight);

    // if the requested dimensions and proxy dimensions don't match,
    // we shouldn't attempt to create the texture
    if ((realWidth != texWidth) || (realHeight != texHeight)) {
        J2dTraceLn2(J2D_TRACE_ERROR,
                    "actual dimensions != requested (actual: w=%d h=%d)",
                    realWidth, realHeight);
        return JNI_FALSE;
    }

    // initialize the texture with some dummy data (this allows us to create
    // a texture object once with 2^n dimensions, and then use
    // glTexSubImage2D() to provide further updates)
    j2d_glGenTextures(1, &texID);
    j2d_glBindTexture(GL_TEXTURE_2D, texID);
    j2d_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     texWidth, texHeight, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    oglsdo->drawableType = OGLSD_TEXTURE;
    oglsdo->xOffset = 0;
    oglsdo->yOffset = 0;
    oglsdo->width = width;
    oglsdo->height = height;
    oglsdo->textureID = texID;
    oglsdo->textureWidth = texWidth;
    oglsdo->textureHeight = texHeight;

    J2dTraceLn3(J2D_TRACE_VERBOSE, "created texture (w=%d h=%d id=%d)",
                width, height, texID);

    return JNI_TRUE;
}

/**
 * Initializes a faux volatile offscreen surface of the given width and height
 * that is actually stored in the backbuffer of a double-buffered onscreen
 * window.  The surface is volatile in the sense that it will be destroyed
 * when the underlying heavyweight window is destroyed.
 *
 * REMIND: this code is not yet fully operational...
 */
JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_OGLSurfaceData_initVolatileBackbuffer
    (JNIEnv *env, jobject oglsd,
     jlong pData, jint width, jint height)
{
    OGLSDOps *oglsdo = (OGLSDOps *)jlong_to_ptr(pData);

    J2dTraceLn2(J2D_TRACE_INFO,
                "in OGLSurfaceData_initVolatileBackbuffer (w=%d h=%d)",
                width, height);

    if (oglsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
        return JNI_FALSE;
    }

    oglsdo->drawableType = OGLSD_VOL_BACKBUFFER;
    // x/yOffset have already been set in initWindow()...
    oglsdo->width = width;
    oglsdo->height = height;
    // REMIND: for some reason, flipping won't work properly on IFB unless we
    //         explicitly use BACK_LEFT rather than BACK...
    oglsdo->activeBuffer = GL_BACK_LEFT;

    // REMIND: need to set peerBuffer

    return JNI_TRUE;
}

/**
 * Initializes a surface in the backbuffer of a given double-buffered
 * onscreen window for use in a BufferStrategy.Flip situation.  The bounds of
 * the backbuffer surface should always be kept in sync with the bounds of
 * the underlying native window.
 */
JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_OGLSurfaceData_initFlipBackbuffer
    (JNIEnv *env, jobject oglsd,
     jlong pData)
{
    OGLSDOps *oglsdo = (OGLSDOps *)jlong_to_ptr(pData);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSurfaceData_initFlipBackbuffer");

    if (oglsdo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "ops are null");
        return JNI_FALSE;
    }

    oglsdo->drawableType = OGLSD_FLIP_BACKBUFFER;
    // x/yOffset have already been set in initWindow()...
    // REMIND: for some reason, flipping won't work properly on IFB unless we
    //         explicitly use BACK_LEFT rather than BACK...
    oglsdo->activeBuffer = GL_BACK_LEFT;

    // REMIND: need to set peerBuffer

    return JNI_TRUE;
}

/**
 * Initializes a small texture tile for use with tiled mask operations (see
 * OGLMaskFill.c for a usage example).  The texture ID for the tile is stored
 * in the given OGLContext.  The tile is initially filled with garbage values,
 * but the tile is updated as needed (via glTexSubImage2D()) with real alpha
 * values used in masking situations.  The internal format for the texture is
 * GL_INTENSITY, which when combined with the GL_MODULATE texture enviornment
 * flag, will effectively multiply the alpha and color components of the
 * source fragment with the alpha value stored in the corresponding location
 * in the mask texture.
 *
 * REMIND: should only be accessible via shared texture context
 */
jint
OGLSD_InitMaskTileTexture(OGLContext *oglc)
{
    GLint sp, sr, rl, align;
    GLclampf priority = 1.0f;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_InitMaskTileTexture");

    j2d_glGenTextures(1, &oglc->maskTextureID);
    j2d_glBindTexture(GL_TEXTURE_2D, oglc->maskTextureID);
    j2d_glPrioritizeTextures(1, &oglc->maskTextureID, &priority);

    // save pixel store parameters (since this method could be invoked after
    // the caller has already set up its pixel store parameters)
    j2d_glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &sp);
    j2d_glGetIntegerv(GL_UNPACK_SKIP_ROWS, &sr);
    j2d_glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rl);
    j2d_glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);

    // set pixel store parameters to default values
    j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    j2d_glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY,
                     OGLSD_MASK_TILE_SIZE, OGLSD_MASK_TILE_SIZE, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    // restore pixel store parameters
    j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, sp);
    j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, sr);
    j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, rl);
    j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, align);

    return SD_SUCCESS;
}

/**
 * Initializes a small texture tile for use with tiled blit operations (see
 * OGLBlitLoops.c and OGLMaskBlit.c for usage examples).  The texture ID for
 * the tile is stored in the given OGLContext.  The tile is initially filled
 * with garbage values, but the tile is updated as needed (via
 * glTexSubImage2D()) with real RGBA values used in tiled blit situations.
 * The internal format for the texture is GL_RGBA, which should be sufficient
 * for storing system memory surfaces of any known format (see PixelFormats
 * for a list of compatible surface formats).
 *
 * REMIND: should only be accessible via shared texture context
 */
jint
OGLSD_InitBlitTileTexture(OGLContext *oglc)
{
    GLint sp, sr, rl, align;
    GLclampf priority = 1.0f;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_InitBlitTileTexture");

    j2d_glGenTextures(1, &oglc->blitTextureID);
    j2d_glBindTexture(GL_TEXTURE_2D, oglc->blitTextureID);
    j2d_glPrioritizeTextures(1, &oglc->blitTextureID, &priority);

    // save pixel store parameters (since this method could be invoked after
    // the caller has already set up its pixel store parameters)
    j2d_glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &sp);
    j2d_glGetIntegerv(GL_UNPACK_SKIP_ROWS, &sr);
    j2d_glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rl);
    j2d_glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);

    // set pixel store parameters to default values
    j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    j2d_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     OGLSD_BLIT_TILE_SIZE, OGLSD_BLIT_TILE_SIZE, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    // restore pixel store parameters
    j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, sp);
    j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, sr);
    j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, rl);
    j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, align);

    return SD_SUCCESS;
}

/**
 * Disposes of all native resources associated with this surface.
 */
static void
OGLSD_Flush(JNIEnv *env, OGLSDOps *oglsdo)
{
    J2dTraceLn1(J2D_TRACE_INFO, "in OGLSD_Flush (type=%d)",
                oglsdo->drawableType);

    if (oglsdo->drawableType == OGLSD_TEXTURE) {
        if (oglsdo->textureID != 0) {
            j2d_glDeleteTextures(1, &oglsdo->textureID);
        }
    } else {
        // dispose windowing system resources (pbuffer, pixmap, etc)
        OGLSD_DisposeOGLSurface(env, oglsdo);
    }

    if (oglsdo->drawableType == OGLSD_WINDOW) {
        if (oglsdo->peerBufferOps != NULL) {
            // REMIND: mark backbuffer lost...
        }
    } else if (oglsdo->drawableType == OGLSD_VOL_BACKBUFFER ||
               oglsdo->drawableType == OGLSD_FLIP_BACKBUFFER)
    {
        // nullify our peer's reference to ourself... this will allow the
        // peer's backbuffer to be reused...
        if (oglsdo->peerBufferOps != NULL) {
            oglsdo->peerBufferOps->peerBufferOps = NULL;
        }
    }
}

/**
 * This is the implementation of the general DisposeFunc defined in
 * SurfaceData.h and used by the Disposer mechanism.  It first flushes all
 * native OpenGL resources and then frees any memory allocated within the
 * native OGLSDOps structure.
 */
void
OGLSD_Dispose(JNIEnv *env, SurfaceDataOps *ops)
{
    OGLSDOps *oglsdo = (OGLSDOps *)ops;
    OGLContext *oglc;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_Dispose");

    // invoke the native threadsafe locking mechanism
    OGLSD_LockImpl(env);

    // first make the shared context for this thread current
    oglc = OGLSD_GetSharedContext(env);
    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not get thread OGL context");
        OGLSD_UnlockImpl(env, OGLSD_NO_FLUSH);
        return;
    }

    // invalidate the current context
    OGLContext_InvalidateCurrentContext(env);

    // flush the native resources
    OGLSD_Flush(env, oglsdo);

    // free all memory allocated within the native ops
    if (oglsdo->privOps != NULL) {
        free(oglsdo->privOps);
    }

    // invoke the native threadsafe unlocking mechanism
    OGLSD_UnlockImpl(env, OGLSD_NO_FLUSH);
}

/**
 * This is the implementation of the general surface LockFunc defined in
 * SurfaceData.h.
 */
jint
OGLSD_Lock(JNIEnv *env,
           SurfaceDataOps *ops,
           SurfaceDataRasInfo *pRasInfo,
           jint lockflags)
{
    OGLSDOps *oglsdo = (OGLSDOps *)ops;
    OGLRIPrivate *priv = (OGLRIPrivate *)&pRasInfo->priv;
    OGLContext *oglc;
    jint ret = SD_SUCCESS;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_Lock");

    OGLSD_LockImpl(env);

    oglc = OGLContext_GetContext(env, oglsdo->sdOps.sdObject);
    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not fetch OGLContext");
        OGLSD_UnlockImpl(env, OGLSD_NO_FLUSH);
        return SD_FAILURE;
    }

    if (oglsdo->drawableType == OGLSD_UNDEFINED ||
        oglsdo->drawableType == OGLSD_TEXTURE)
    {
        J2dTraceLn(J2D_TRACE_ERROR,
                   "cannot lock a texture or undefined surface");
        OGLSD_UnlockImpl(env, OGLSD_NO_FLUSH);
        return SD_FAILURE;
    }

    if ((lockflags & SD_LOCK_RD_WR) && (lockflags & SD_LOCK_FASTEST)) {
        ret = SD_SLOWLOCK;
    }

    if ((lockflags & SD_LOCK_WRITE) && !oglsdo->sdOps.dirty) {
	SurfaceData_MarkDirty(env, &oglsdo->sdOps);
    }

    priv->context = oglc;
    priv->lockFlags = lockflags;

    return ret;
}

/**
 * This is the implementation of the general GetRasInfoFunc defined in
 * SurfaceData.h.
 */
void
OGLSD_GetRasInfo(JNIEnv *env,
                 SurfaceDataOps *ops,
                 SurfaceDataRasInfo *pRasInfo)
{
    OGLSDOps *oglsdo = (OGLSDOps *)ops;
    OGLRIPrivate *priv = (OGLRIPrivate *)&pRasInfo->priv;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_GetRasInfo");

    if (priv->lockFlags & SD_LOCK_RD_WR) {
        int x, y, w, h;
        x = pRasInfo->bounds.x1;
        y = pRasInfo->bounds.y1;
        w = pRasInfo->bounds.x2 - x;
        h = pRasInfo->bounds.y2 - y;

        // initialize temporary pixel buffer
        if ((w == 1) && (h == 1)) {
            priv->pixels = &priv->singlePixel;
        } else {
            priv->pixels = malloc(w * h * sizeof(GLuint));
            if (priv->pixels == NULL) {
                pRasInfo->rasBase = NULL;
                pRasInfo->pixelStride = 0;
                pRasInfo->scanStride = 0;
                return;
            }
        }

        if (priv->lockFlags & SD_LOCK_NEED_PIXELS) {
            int sy = oglsdo->height - y - 1;
            int dy = 0;

            // we must read one scanline at a time because there is no way
            // to read starting at the top-left corner of the source region
            while (dy < h) {
                j2d_glPixelStorei(GL_PACK_SKIP_ROWS, dy);
                j2d_glReadPixels(x, sy, w, 1,
                                 GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                                 priv->pixels);
                sy--;
                dy++;
            }
        }

        pRasInfo->rasBase = (GLuint *)priv->pixels - (y * w) - x;
        pRasInfo->pixelStride = 4;
        pRasInfo->scanStride = w * 4;
    } else {
        // they didn't lock for anything, so we won't give them anything
        pRasInfo->rasBase = NULL;
        pRasInfo->pixelStride = 0;
        pRasInfo->scanStride = 0;
    }
}

/**
 * This is the implementation of the general surface UnlockFunc defined in
 * SurfaceData.h.
 */
void
OGLSD_Unlock(JNIEnv *env,
             SurfaceDataOps *ops,
             SurfaceDataRasInfo *pRasInfo)
{
    OGLSDOps *oglsdo = (OGLSDOps *)ops;
    OGLRIPrivate *priv = (OGLRIPrivate *)&pRasInfo->priv;
    OGLContext *oglc = priv->context;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSD_Unlock");

    if (priv->lockFlags & SD_LOCK_WRITE) {
        int x, y, w, h;
        x = pRasInfo->bounds.x1;
        y = pRasInfo->bounds.y1;
        w = pRasInfo->bounds.x2 - x;
        h = pRasInfo->bounds.y2 - y;

        j2d_glRasterPos2i(0, 0);
        j2d_glBitmap(0, 0, 0, 0, (GLfloat)x, (GLfloat)-y, NULL);

        // REMIND: Pixel values in priv->pixels are assumed to be of type
        //         IntRgb, but the alpha byte may not be fully opaque;
        //         this bias step ensures that we write a fully opaque pixel
        //         into the framebuffer.  We may want to change this behavior
        //         if we eventually allow pixels with an alpha channel.
        j2d_glPixelTransferf(GL_ALPHA_BIAS, 1.0f);
        j2d_glPixelZoom(1.0, -1.0);

        j2d_glDrawPixels(w, h,
                         GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                         priv->pixels);

        j2d_glPixelTransferf(GL_ALPHA_BIAS, 0.0f);
        j2d_glPixelZoom(1.0, 1.0);
    }

    if ((priv->pixels != NULL) && (priv->pixels != &priv->singlePixel)) {
        free(priv->pixels);
    }

    OGLContext_Flush(env, oglc);
    OGLSD_UnlockImpl(env, OGLSD_NO_FLUSH);
}

JNIEXPORT void JNICALL 
Java_sun_java2d_opengl_OGLSurfaceData_flush(JNIEnv *env, jobject oglsd,
                                            jlong pData)
{
    OGLSDOps *oglsdo = (OGLSDOps *)jlong_to_ptr(pData);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLSurfaceData_flush");

    if (oglsdo != NULL) {
        // invoke the surface flush
        OGLSD_Flush(env, oglsdo);
    }
}

#endif /* !HEADLESS */
