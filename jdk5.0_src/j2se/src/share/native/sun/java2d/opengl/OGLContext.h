/*
 * @(#)OGLContext.h	1.4 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLContext_h_Included
#define OGLContext_h_Included

#include "sun_java2d_opengl_OGLContext.h"

#include "J2D_GL/gl.h"

/**
 * The OGLExtInfo structure contains flags that indicate whether a particular
 * extension (or logical group of extensions) is available for a
 * GraphicsConfiguration.
 *
 *     jboolean imaging;
 * If true, indicates the presence of the GL_ARB_imaging subset extension.
 *
 *     jboolean blendPremult;
 * If true, indicates the presence of both the GL_EXT_blend_func_separate and
 * the GL_SUN_blend_src_mult_dst_alpha extensions.
 *
 *     jboolean multitexture;
 * If true, indicates the presence of the GL_ARB_multitexture extension.
 *
 *     jboolean texNonPow2;
 * If true, indicates the presence of the GL_ARB_texture_non_power_of_two
 * extension.
 */
typedef struct {
    jboolean imaging;
    jboolean blendPremult;
    jboolean multitexture;
    jboolean texNonPow2;
} OGLExtInfo;

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
 * The extInfo field is a pointer to the extension availability information
 * stored in the native GraphicsConfig structure.  The other fields are
 * simply cached values of various elements of the context state, typically
 * used in the OGLContext.set*() methods.
 *
 * REMIND: the maskTextureID and blitTextureID fields should only be used by
 *         the shared texture context (TBD)...
 */
typedef struct {
    void       *ctxInfo;
    OGLExtInfo *extInfo;
    jboolean   onJED;
    jint       compState;
    jfloat     extraAlpha;
    jint       xorPixel;
    GLdouble   *xformMatrix;
    GLuint     maskTextureID;
    GLuint     blitTextureID;
} OGLContext;

/**
 * The following flags help the internals of OGLContext.validate() determine
 * the appropriate (meaning correct, or optimal) code path when setting up
 * the current context.  The flags can be bitwise OR'd together as needed.
 *
 *     OGLC_NO_CONTEXT_FLAGS
 * Indicates that no flags are needed; take all default code paths.
 *
 *     OGLC_USE_EXTRA_ALPHA
 * Indicates that the source surface should have the "extra alpha" value (if
 * present in the given AlphaComposite object) applied to it.  This flag is
 * only relevant when copying system memory ("Sw") surface to OpenGL surface.
 * [Note that pixel values, used in simple rendering operations, will always
 * have the extra alpha value applied before setting the current OpenGL color
 * state.]
 *
 *     OGLC_SRC_IS_PREMULT
 * Indicates that the source surface (or pixel value, if it is a simple
 * rendering operation) has color components that have already been
 * pre-multiplied by their corresponding alpha value.  If this flag is not
 * present, certain code paths may be taken to ensure that the surface (or
 * pixel value) is first pre-multiplied before further processing takes place.
 *
 *     OGLC_SRC_IS_OPAQUE
 * Indicates that the source surface (or pixel value, if it is a simple
 * rendering operation) is opaque (has an alpha value of 1.0).  If this flag
 * is present, it allows us to disable blending in certain situations in
 * order to improve performance.
 */
#define OGLC_NO_CONTEXT_FLAGS \
    sun_java2d_opengl_OGLContext_NO_CONTEXT_FLAGS
#define OGLC_USE_EXTRA_ALPHA  \
    sun_java2d_opengl_OGLContext_USE_EXTRA_ALPHA
#define OGLC_SRC_IS_PREMULT   \
    sun_java2d_opengl_OGLContext_SRC_IS_PREMULT
#define OGLC_SRC_IS_OPAQUE    \
    sun_java2d_opengl_OGLContext_SRC_IS_OPAQUE

OGLContext * OGLContext_GetContext(JNIEnv *env, jobject dstData);
void OGLContext_InvalidateCurrentContext(JNIEnv *env);
void OGLContext_InvokeGLFlush(JNIEnv *env);
void OGLContext_Flush(JNIEnv *env, OGLContext *oglc);

jboolean OGLContext_IsExtensionAvailable(const char *extString, char *extName);
void OGLContext_GetExtensionInfo(OGLExtInfo *extInfo);
jboolean OGLContext_IsVersionSupported(const unsigned char *versionstr);

#endif /* OGLContext_h_Included */
