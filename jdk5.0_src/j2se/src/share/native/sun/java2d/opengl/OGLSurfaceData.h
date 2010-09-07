/*
 * @(#)OGLSurfaceData.h	1.11 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLSurfaceData_h_Included
#define OGLSurfaceData_h_Included

#include "java_awt_image_AffineTransformOp.h"
#include "sun_java2d_opengl_OGLSurfaceData.h"

#include "J2D_GL/gl.h"
#include "SurfaceData.h"
#include "Trace.h"
#include "OGLContext.h"
#include "OGLFuncs.h"

typedef struct _OGLSDOps OGLSDOps;

/**
 * The OGLPixelFormat structure contains all the information OpenGL needs to
 * know when copying from or into a particular system memory image buffer (via
 * glDrawPixels(), glReadPixels, glTexSubImage2D(), etc).
 *
 *     GLenum format;
 * The pixel format parameter used in glDrawPixels() and other similar calls.
 * Indicates the component ordering for each pixel (e.g. GL_BGRA).
 *
 *     GLenum type;
 * The pixel data type parameter used in glDrawPixels() and other similar
 * calls.  Indicates the data type for an entire pixel or for each component
 * in a pixel (e.g. GL_UNSIGNED_BYTE with GL_BGR means a pixel consists of
 * 3 unsigned byte components, blue first, then green, then red;
 * GL_UNSIGNED_INT_8_8_8_8_REV with GL_BGRA means a pixel consists of 1
 * unsigned integer comprised of four byte components, alpha first, then red,
 * then green, then blue).
 *
 *     jint alignment;
 * The byte alignment parameter used in glPixelStorei(GL_UNPACK_ALIGNMENT).  A
 * value of 4 indicates that each pixel starts on a 4-byte aligned region in
 * memory, and so on.  This alignment parameter helps OpenGL speed up pixel
 * transfer operations by transferring memory in aligned blocks.
 *
 *     jboolean hasAlpha;
 * If true, indicates that this pixel format contains an alpha component.
 *
 *     jboolean isPremult;
 * If true, indicates that this pixel format contains color components that
 * have been pre-multiplied by their corresponding alpha component.
 *
 *     jint lockFlags;
 * Contains any flags that are relevant when locking a system memory surface
 * of this pixel format.  For example, if each pixel is actually an index into
 * a color map, lockFlags should contain SD_LOCK_LUT to indicate that the LUT
 * should be locked.
 */
typedef struct {
    GLenum   format;
    GLenum   type;
    jint     alignment;
    jboolean hasAlpha;
    jboolean isPremult;
    jint     lockFlags;
} OGLPixelFormat;

/**
 * The OGLRIPrivate structure contains information needed during a
 * Lock/GetRasInfo/Unlock cycle.  This structure is stored in the
 * SurfaceDataRasInfo "priv" field.  See the definition of SurfaceDataRasInfo
 * in SurfaceData.h for more information.
 */
typedef struct {
    OGLContext *context;
    void       *pixels;
    GLuint     singlePixel;
    jint       lockFlags;
} OGLRIPrivate;

/**
 * The OGLSDOps structure describes a native OpenGL surface and contains all
 * information pertaining to the native surface.  Some information about
 * the more important/different fields:
 *
 *     void *privOps;
 * Pointer to native-specific (GLX, WGL, etc.) SurfaceData info, such as the
 * native Drawable handle and GraphicsConfig data.
 *
 *     OGLSDOps *peerBufferOps;
 * Pointer to the native OGLSDOps for this surface's peer surface.  For
 * example, if this surface is of type OGLSD_WINDOW and we have a
 * double-buffered GraphicsConfig, it is possible to have another OGLSDOps
 * for the backbuffer of type OGLSD_FLIP_BACKBUFFER.  This OGLSDOps can
 * contain a pointer to the OGLSDOps for that backbuffer surface, and vice
 * versa.  This helps the surfaces manage their resources, such as when the
 * main window surface is disposed and we want to dispose of its peer
 * (backbuffer) surface.
 *
 *     jint drawableType;
 * The surface type; can be any one of the surface type constants defined
 * below (OGLSD_WINDOW, OGLSD_TEXTURE, etc).
 *
 *     GLenum activeBuffer;
 * Can be either GL_FRONT if this is the front buffer surface of an onscreen
 * window or a pbuffer surface, or GL_BACK if this is the backbuffer surface
 * of an onscreen window.
 *
 *     jint x/yOffset
 * The offset in pixels of the OpenGL viewport origin from the lower-left
 * corner of the heavyweight drawable.  For example, a top-level frame on
 * Windows XP has lower-left insets of (4,4).  The OpenGL viewport origin
 * would typically begin at the lower-left corner of the client region (inside
 * the frame decorations), but AWT/Swing will take the insets into account
 * when rendering into that window.  So in order to account for this, we
 * need to adjust the OpenGL viewport origin by an x/yOffset of (-4,-4).  On
 * X11, top-level frames typically don't have this insets issue, so their
 * x/yOffset would be (0,0) (the same applies to pbuffers).  Another
 * possible usage of the x/yOffset fields is with OGLSD_VOL_BACKBUFFER
 * surfaces, in order to reposition the virtual surface so that its
 * lower-left corner correlates to the lower-left corner of the "viewable"
 * region of the backbuffer.
 *
 *     jint width/height;
 * The cached surface bounds.  For offscreen surface types (OGLSD_PBUFFER,
 * OGLSD_TEXTURE, etc.) these values must remain constant.  Onscreen window
 * surfaces (OGLSD_WINDOW, OGLSD_FLIP_BACKBUFFER, etc.) may have their
 * bounds changed in response to a programmatic or user-initiated event, so
 * these values represent the last known dimensions.  To determine the true
 * current bounds of this surface, query the native Drawable through the
 * privOps field.
 *
 *     GLuint textureID;
 * The texture object handle, as generated by glGenTextures().  If this value
 * is zero, the texture has not yet been initialized.
 *
 *     jint textureWidth/Height;
 * The actual bounds of the texture object for this surface.  If the
 * GL_ARB_texture_non_power_of_two extension is not present, the dimensions
 * of an OpenGL texture object must be a power-of-two (e.g. 64x32 or 128x512).
 * The texture image that we care about has dimensions specified by the width
 * and height fields in this OGLSDOps structure.  For example, if the image
 * to be stored in the texture has dimensions 115x47, the actual OpenGL
 * texture we allocate will have dimensions 128x64 to meet the pow2
 * restriction.  The image bounds within the texture can be accessed using
 * floating point texture coordinates in the range [0.0,1.0].
 */
struct _OGLSDOps {
    SurfaceDataOps               sdOps;
    void                         *privOps;
    OGLSDOps                     *peerBufferOps;
    jint                         drawableType;
    GLenum                       activeBuffer;
    jboolean                     isPremult;
    jint                         xOffset;
    jint                         yOffset;
    jint                         width;
    jint                         height;
    GLuint                       textureID;
    jint                         textureWidth;
    jint                         textureHeight;
};

/**
 * The following convenience macros are used when rendering rectangles (either
 * a single rectangle, or a whole series of them).  To render a single
 * rectangle, simply invoke the GLRECT() macro.  To render a whole series of
 * rectangles, such as spans in a complex shape, first invoke GLRECT_BEGIN(),
 * then invoke the appropriate inner loop macro (either XYXY or XYWH) for
 * each rectangle, and finally invoke GLRECT_END() to notify OpenGL that the
 * vertex list is complete.  Care should be taken to avoid calling OpenGL
 * commands (besides GLRECT_BODY_*()) inside the BEGIN/END pair.
 */

#define GLRECT_BEGIN j2d_glBegin(GL_QUADS)

#define GLRECT_BODY_XYXY(x1, y1, x2, y2) \
    do { \
        j2d_glVertex2i(x1, y1); \
        j2d_glVertex2i(x2, y1); \
        j2d_glVertex2i(x2, y2); \
        j2d_glVertex2i(x1, y2); \
    } while (0)

#define GLRECT_BODY_XYWH(x, y, w, h) \
    GLRECT_BODY_XYXY(x, y, (x) + (w), (y) + (h))

#define GLRECT_END j2d_glEnd()

/**
 * REMIND: this is an alternate implementation of the GLRECT macro that uses
 *         the glRecti() method, which may be fast, but probably not fast
 *         enough in batch situations (leaving it here for performance
 *         comparisons in the future)...
 */
/*
#define GLRECT_BEGIN
#define GLRECT_BODY_XYWH(x, y, w, h) j2d_glRecti(x, y, (x) + (w), (y) + (h))
#define GLRECT_END
*/

#define GLRECT(x, y, w, h) \
    do { \
        GLRECT_BEGIN; \
        GLRECT_BODY_XYWH(x, y, w, h); \
        GLRECT_END; \
    } while (0)

/**
 * These are shorthand names for the surface type constants defined in
 * OGLSurfaceData.java.
 */
#define OGLSD_UNDEFINED       sun_java2d_opengl_OGLSurfaceData_UNDEFINED
#define OGLSD_WINDOW          sun_java2d_opengl_OGLSurfaceData_WINDOW
#define OGLSD_PIXMAP          sun_java2d_opengl_OGLSurfaceData_PIXMAP
#define OGLSD_PBUFFER         sun_java2d_opengl_OGLSurfaceData_PBUFFER
#define OGLSD_TEXTURE         sun_java2d_opengl_OGLSurfaceData_TEXTURE
#define OGLSD_VOL_BACKBUFFER  sun_java2d_opengl_OGLSurfaceData_VOL_BACKBUFFER
#define OGLSD_FLIP_BACKBUFFER sun_java2d_opengl_OGLSurfaceData_FLIP_BACKBUFFER

/**
 * These two constants determine the size of the shared tile textures used
 * by a number of image rendering methods.  For example, the mask tile texture
 * will have dimensions with width OGLSD_MASK_TILE_SIZE and height
 * OGLSD_MASK_TILE_SIZE (the tile will always be square).
 */
#define OGLSD_MASK_TILE_SIZE 32
#define OGLSD_BLIT_TILE_SIZE 32

/**
 * These are shorthand names for the filtering method constants used by
 * image transform methods.
 */
#define OGLSD_XFORM_DEFAULT 0
#define OGLSD_XFORM_NEAREST_NEIGHBOR \
    java_awt_image_AffineTransformOp_TYPE_NEAREST_NEIGHBOR
#define OGLSD_XFORM_BILINEAR \
    java_awt_image_AffineTransformOp_TYPE_BILINEAR

/**
 * The following flags are used as parameters to the OGLSD_UnlockImpl()
 * function.
 *
 *     OGLSD_NO_FLUSH
 * Indicates that the native windowing layer should be unlocked, but there's
 * no need to flush the OpenGL pipeline.
 *
 *     OGLSD_FLUSH_NOW
 * Indicates that the OpenGL context that is current to the calling thread
 * should be flushed immediately with a call to glFlush().  Use this when
 * we are not on the Java EventDispatchThread, because we have no control
 * over when the next flush might occur.
 *
 *     OGLSD_FLUSH_ON_JED
 * Indicates that the calling thread is the Java-level EDT and the OpenGL
 * pipeline should be flushed whenever appropriate.  Use this flag when we
 * know we are on the Java EDT, and then an asynchronous flush event will
 * be posted to the Java EventQueue, which will eventually flush the pipeline
 * after pending events are consumed.
 */
#define OGLSD_NO_FLUSH     0
#define OGLSD_FLUSH_NOW    1
#define OGLSD_FLUSH_ON_JED 2

/**
 * The following constants define the inner and outer bounds of the
 * accelerated glyph cache.
 */
#define OGLSD_CACHE_WIDTH       512
#define OGLSD_CACHE_HEIGHT      512
#define OGLSD_CACHE_CELL_WIDTH  16
#define OGLSD_CACHE_CELL_HEIGHT 16

/**
 * Exported methods.
 */
jint OGLSD_Lock(JNIEnv *env,
                SurfaceDataOps *ops, SurfaceDataRasInfo *pRasInfo,
                jint lockflags);
void OGLSD_GetRasInfo(JNIEnv *env,
                      SurfaceDataOps *ops, SurfaceDataRasInfo *pRasInfo);
void OGLSD_Unlock(JNIEnv *env,
                  SurfaceDataOps *ops, SurfaceDataRasInfo *pRasInfo);
void OGLSD_Dispose(JNIEnv *env, SurfaceDataOps *ops);

jint OGLSD_NextPowerOfTwo(jint val, jint max);
jint OGLSD_InitMaskTileTexture(OGLContext *oglc);
jint OGLSD_InitBlitTileTexture(OGLContext *oglc);

#endif /* OGLSurfaceData_h_Included */
