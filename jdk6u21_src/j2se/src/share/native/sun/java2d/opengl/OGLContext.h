/*
 * @(#)OGLContext.h	1.16 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLContext_h_Included
#define OGLContext_h_Included

#include "sun_java2d_pipe_BufferedContext.h"
#include "sun_java2d_opengl_OGLContext.h"
#include "sun_java2d_opengl_OGLContext_OGLContextCaps.h"

#include "j2d_md.h"
#include "J2D_GL/gl.h"
#include "OGLSurfaceData.h"

/**
 * The OGLBlendRule structure encapsulates the two enumerated values that
 * comprise a given Porter-Duff blending (compositing) rule.  For example,
 * the "SrcOver" rule can be represented by:
 *     rule.src = GL_ONE;
 *     rule.dst = GL_ONE_MINUS_SRC_ALPHA;
 *
 *     GLenum src;
 * The constant representing the source factor in this Porter-Duff rule.
 *
 *     GLenum dst;
 * The constant representing the destination factor in this Porter-Duff rule.
 */
typedef struct {
    GLenum src;
    GLenum dst;
} OGLBlendRule;

/**
 * The OGLContext structure contains cached state relevant to the native
 * OpenGL context stored within the native ctxInfo field.  Each Java-level
 * OGLContext object is associated with a native-level OGLContext structure.
 * The caps field is a bitfield that expresses the capabilities of the
 * GraphicsConfig associated with this context (see OGLContext.java for
 * the definitions of each capability bit).  The other fields are
 * simply cached values of various elements of the context state, typically
 * used in the OGLContext.set*() methods.
 *
 * Note that the textureFunction field is implicitly set to zero when the
 * OGLContext is created.  The acceptable values (e.g. GL_MODULATE,
 * GL_REPLACE) for this field are never zero, which means we will always
 * validate the texture function state upon the first call to the
 * OGLC_UPDATE_TEXTURE_FUNCTION() macro.
 */
typedef struct {
    void       *ctxInfo;
    jint       caps;
    jint       compState;
    jfloat     extraAlpha;
    jint       xorPixel;
    jint       pixel;
    jubyte     r;
    jubyte     g;
    jubyte     b;
    jubyte     a;
    jint       paintState;
    jboolean   useMask;
    GLdouble   *xformMatrix;
    GLuint     blitTextureID;
    GLint      textureFunction;
} OGLContext;

/**
 * See BufferedContext.java for more on these flags...
 */
#define OGLC_NO_CONTEXT_FLAGS \
    sun_java2d_pipe_BufferedContext_NO_CONTEXT_FLAGS
#define OGLC_SRC_IS_OPAQUE    \
    sun_java2d_pipe_BufferedContext_SRC_IS_OPAQUE
#define OGLC_USE_MASK         \
    sun_java2d_pipe_BufferedContext_USE_MASK

/**
 * See OGLContext.java for more on these flags.
 */
#define CAPS_EMPTY           \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_EMPTY
#define CAPS_RT_PLAIN_ALPHA  \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_RT_PLAIN_ALPHA
#define CAPS_RT_TEXTURE_ALPHA       \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_RT_TEXTURE_ALPHA
#define CAPS_RT_TEXTURE_OPAQUE      \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_RT_TEXTURE_OPAQUE
#define CAPS_MULTITEXTURE    \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_MULTITEXTURE
#define CAPS_TEXNONPOW2      \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_TEXNONPOW2
#define CAPS_TEXNONSQUARE    \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_TEXNONSQUARE
#define CAPS_PS20            \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_PS20
#define CAPS_PS30            \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_PS30
#define LAST_SHARED_CAP      \
    sun_java2d_opengl_OGLContext_OGLContextCaps_LAST_SHARED_CAP
#define CAPS_EXT_FBOBJECT    \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_EXT_FBOBJECT
#define CAPS_STORED_ALPHA    \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_STORED_ALPHA
#define CAPS_DOUBLEBUFFERED  \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_DOUBLEBUFFERED
#define CAPS_EXT_LCD_SHADER  \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_EXT_LCD_SHADER
#define CAPS_EXT_BIOP_SHADER \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_EXT_BIOP_SHADER
#define CAPS_EXT_GRAD_SHADER \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_EXT_GRAD_SHADER
#define CAPS_EXT_TEXRECT     \
    sun_java2d_opengl_OGLContext_OGLContextCaps_CAPS_EXT_TEXRECT

/**
 * Evaluates to true if the given capability bitmask is present for the
 * given OGLContext.  Note that only the constant name needs to be passed as
 * a parameter, as this macro will automatically prepend the full package
 * and class name to the constant name.
 */
#define OGLC_IS_CAP_PRESENT(oglc, cap) (((oglc)->caps & (cap)) != 0)

/**
 * This constant determines the size of the shared tile texture used
 * by a number of image rendering methods.  For example, the blit tile texture
 * will have dimensions with width OGLC_BLIT_TILE_SIZE and height
 * OGLC_BLIT_TILE_SIZE (the tile will always be square).
 */
#define OGLC_BLIT_TILE_SIZE 32

/**
 * Helper macros that update the current texture function state only when
 * it needs to be changed, which helps reduce overhead for small texturing
 * operations.  The filter state is set on a per-context (not per-texture)
 * basis; for example, if we apply one texture using GL_MODULATE followed by
 * another texture using GL_MODULATE (under the same context), there is no
 * need to set the texture function the second time, as that would be
 * redundant.
 */
#define OGLC_INIT_TEXTURE_FUNCTION(oglc, func)                      \
    do {                                                            \
        j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (func)); \
        (oglc)->textureFunction = (func);                           \
    } while (0)

#define OGLC_UPDATE_TEXTURE_FUNCTION(oglc, func)    \
    do {                                            \
        if ((oglc)->textureFunction != (func)) {    \
            OGLC_INIT_TEXTURE_FUNCTION(oglc, func); \
        }                                           \
    } while (0)

/**
 * Exported methods.
 */
OGLContext *OGLContext_SetSurfaces(JNIEnv *env, jlong pSrc, jlong pDst);
void OGLContext_ResetClip(OGLContext *oglc);
void OGLContext_SetRectClip(OGLContext *oglc, OGLSDOps *dstOps,
                            jint x1, jint y1, jint x2, jint y2);
void OGLContext_BeginShapeClip(OGLContext *oglc);
void OGLContext_EndShapeClip(OGLContext *oglc, OGLSDOps *dstOps);
void OGLContext_SetExtraAlpha(jfloat ea);
void OGLContext_ResetComposite(OGLContext *oglc);
void OGLContext_SetAlphaComposite(OGLContext *oglc,
                                  jint rule, jfloat extraAlpha, jint flags);
void OGLContext_SetXorComposite(OGLContext *oglc, jint xorPixel);
void OGLContext_ResetTransform(OGLContext *oglc);
void OGLContext_SetTransform(OGLContext *oglc,
                             jdouble m00, jdouble m10,
                             jdouble m01, jdouble m11,
                             jdouble m02, jdouble m12);

jboolean OGLContext_InitBlitTileTexture(OGLContext *oglc);
GLuint OGLContext_CreateBlitTexture(GLenum internalFormat, GLenum pixelFormat,
                                    GLuint width, GLuint height);

void OGLContext_DestroyContextResources(OGLContext *oglc);

jboolean OGLContext_IsExtensionAvailable(const char *extString, char *extName);
void OGLContext_GetExtensionInfo(JNIEnv *env, jint *caps);
jboolean OGLContext_IsVersionSupported(const unsigned char *versionstr);

GLhandleARB OGLContext_CreateFragmentProgram(const char *fragmentShaderSource);

#endif /* OGLContext_h_Included */
