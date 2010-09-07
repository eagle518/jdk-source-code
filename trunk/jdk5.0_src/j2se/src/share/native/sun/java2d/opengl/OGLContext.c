/*
 * @(#)OGLContext.c	1.6 04/03/31
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef HEADLESS

#include <stdlib.h>
#include <string.h>

#include "sun_java2d_SunGraphics2D.h"

#include "jlong.h"
#include "jni_util.h"
#include "OGLContext.h"
#include "OGLSurfaceData.h"
#include "GraphicsPrimitiveMgr.h"
#include "Region.h"

static jclass oglcClass;
static jmethodID invokeFlushID;
static jmethodID getContextID;
static jmethodID invalidateID;

extern jboolean needGLFlush;
extern void OGLSD_OutputFlush(JNIEnv *env);

/**
 * This table contains the standard blending rules (or Porter-Duff compositing
 * factors) used in glBlendFunc(), indexed by the rule constants from the
 * AlphaComposite class.
 */
OGLBlendRule StdBlendRules[] = {
    { GL_ZERO,                GL_ZERO                }, /* 0 - Nothing      */
    { GL_ZERO,                GL_ZERO                }, /* 1 - RULE_Clear   */
    { GL_ONE,                 GL_ZERO                }, /* 2 - RULE_Src     */
    { GL_ONE,                 GL_ONE_MINUS_SRC_ALPHA }, /* 3 - RULE_SrcOver */
    { GL_ONE_MINUS_DST_ALPHA, GL_ONE                 }, /* 4 - RULE_DstOver */
    { GL_DST_ALPHA,           GL_ZERO                }, /* 5 - RULE_SrcIn   */
    { GL_ZERO,                GL_SRC_ALPHA           }, /* 6 - RULE_DstIn   */
    { GL_ONE_MINUS_DST_ALPHA, GL_ZERO                }, /* 7 - RULE_SrcOut  */
    { GL_ZERO,                GL_ONE_MINUS_SRC_ALPHA }, /* 8 - RULE_DstOut  */
    { GL_ZERO,                GL_ONE                 }, /* 9 - RULE_Dst     */
    { GL_DST_ALPHA,           GL_ONE_MINUS_SRC_ALPHA }, /*10 - RULE_SrcAtop */
    { GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA           }, /*11 - RULE_DstAtop */
    { GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA }, /*12 - RULE_AlphaXor*/
};

/**
 * This table contains the "modified" blending rules (or Porter-Duff
 * compositing factors) used in glBlendFuncSeparateEXT(), indexed by the
 * rule constants from the AlphaComposite class.  These rules should be used
 * only when both the GL_EXT_blend_func_separate and
 * GL_SUN_blend_src_mult_dst_alpha extensions are available.  The rules are
 * applied only to the color components in order to effectively multiply the
 * source alpha value by the color components during the blending step.  Note
 * that the StdBlendRules should be applied to the alpha component in
 * glBlendFuncSeparateEXT() to avoid multiplying the alpha value by itself.
 *
 * [See OGLContext_SetBlendFunc() for a usage example of ModBlendRules.]
 */
OGLBlendRule ModBlendRules[] = {
    { GL_ZERO,
      GL_ZERO                                        }, /* 0 - Nothing      */
    { GL_ZERO,
      GL_ZERO                                        }, /* 1 - RULE_Clear   */
    { GL_SRC_ALPHA,
      GL_ZERO                                        }, /* 2 - RULE_Src     */
    { GL_SRC_ALPHA,
      GL_ONE_MINUS_SRC_ALPHA                         }, /* 3 - RULE_SrcOver */
    { GL_SRC_ALPHA_MULT_ONE_MINUS_DST_ALPHA_SUN,
      GL_ONE                                         }, /* 4 - RULE_DstOver */
    { GL_SRC_ALPHA_MULT_DST_ALPHA_SUN,
      GL_ZERO                                        }, /* 5 - RULE_SrcIn   */
    { GL_ZERO,
      GL_SRC_ALPHA                                   }, /* 6 - RULE_DstIn   */
    { GL_SRC_ALPHA_MULT_ONE_MINUS_DST_ALPHA_SUN,
      GL_ZERO                                        }, /* 7 - RULE_SrcOut  */
    { GL_ZERO,
      GL_ONE_MINUS_SRC_ALPHA                         }, /* 8 - RULE_DstOut  */
    { GL_ZERO,
      GL_ONE                                         }, /* 9 - RULE_Dst     */
    { GL_SRC_ALPHA_MULT_DST_ALPHA_SUN,
      GL_ONE_MINUS_SRC_ALPHA                         }, /*10 - RULE_SrcAtop */
    { GL_SRC_ALPHA_MULT_ONE_MINUS_DST_ALPHA_SUN,
      GL_SRC_ALPHA                                   }, /*11 - RULE_DstAtop */
    { GL_SRC_ALPHA_MULT_ONE_MINUS_DST_ALPHA_SUN,
      GL_ONE_MINUS_SRC_ALPHA                         }, /*12 - RULE_AlphaXor*/
};

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLContext_initIDs(JNIEnv *env, jclass oglc)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_initIDs");

    if (sizeof(OGLRIPrivate) > SD_RASINFO_PRIVATE_SIZE) {
        JNU_ThrowInternalError(env, "Private RasInfo structure too large!");
        return;
    }

    oglcClass = (*env)->NewGlobalRef(env, oglc);
    invokeFlushID = (*env)->GetStaticMethodID(env, oglc,
                                              "invokeNativeGLFlush", "()V");
    getContextID = (*env)->GetStaticMethodID(env, oglc,
                                    "getContext",
                                    "(Lsun/java2d/opengl/OGLSurfaceData;)J");
    invalidateID = (*env)->GetStaticMethodID(env, oglc,
                                             "invalidateCurrentContext",
                                             "()V");
}

/**
 * Initializes the viewport and projection matrix, effectively positioning
 * the origin at the top-left corner of the surface.  This allows Java 2D
 * coordinates to be passed directly to OpenGL, which is typically based on
 * a bottom-right coordinate system.  This method also sets the appropriate
 * read and draw buffers.
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLContext_setViewport(JNIEnv *env, jobject oc,
                                              jlong pSrc, jlong pDst)
{
    OGLSDOps *srcOps = (OGLSDOps *)jlong_to_ptr(pSrc);
    OGLSDOps *dstOps = (OGLSDOps *)jlong_to_ptr(pDst);
    jint width = dstOps->width;
    jint height = dstOps->height;

    J2dTraceLn4(J2D_TRACE_INFO,
                "in OGLContext_setViewport (w=%d h=%d read=%s draw=%s)",
                width, height,
                srcOps->activeBuffer == GL_FRONT ? "front" : "back",
                dstOps->activeBuffer == GL_FRONT ? "front" : "back");

    j2d_glViewport(dstOps->xOffset, dstOps->yOffset,
                   (GLsizei)width, (GLsizei)height);
    j2d_glMatrixMode(GL_PROJECTION);
    j2d_glLoadIdentity();
    j2d_glOrtho(0.0, (GLdouble)width, (GLdouble)height, 0.0, -1.0, 1.0);

    j2d_glReadBuffer(srcOps->activeBuffer);
    j2d_glDrawBuffer(dstOps->activeBuffer);
}

/**
 * Initializes the OpenGL clip state.  If the clip parameter is null, all
 * clipping (both scissor and stencil) is disabled.  If the Region is
 * rectangular, the OpenGL scissor bounds are updated to the Region bounds.
 * If the Region is complex, the Region spans are "rendered" into the stencil
 * buffer.  The stencil buffer is first cleared, then the stencil func and op
 * are setup so that when we render the clip spans, nothing is rendered into
 * the color buffer, but for each pixel that would be rendered, a value of one
 * is placed into that location in the stencil buffer.  With stenciling
 * enabled, pixels will only be rendered into the color buffer if the
 * corresponding value at that (x,y) location in the stencil buffer.
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLContext_setClip(JNIEnv *env, jobject oc,
                                          jlong pDst, jobject clip,
                                          jboolean isRect,
                                          jint x1, jint y1, jint x2, jint y2)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_setClip");

    if (clip == NULL) {
        J2dTraceLn(J2D_TRACE_VERBOSE, "disabling clip");
        j2d_glDisable(GL_SCISSOR_TEST);
        j2d_glDisable(GL_STENCIL_TEST);
    } else {
        if (isRect) {
            // simple rectangular clip; using scissor test
            OGLSDOps *dstOps = (OGLSDOps *)jlong_to_ptr(pDst);
            jint width = x2 - x1;
            jint height = y2 - y1;

            if ((width < 0) || (height < 0)) {
                // use an empty scissor rectangle when the region is empty
                width = 0;
                height = 0;
            }

            J2dTraceLn4(J2D_TRACE_VERBOSE,
                        "enabling rectangular clip (x=%d y=%d w=%d h=%d)",
                        x1, y1, width, height);

            j2d_glDisable(GL_STENCIL_TEST);
            j2d_glEnable(GL_SCISSOR_TEST);

            // the scissor rectangle is specified using the lower-left
            // origin of the clip region (in the framebuffer's coordinate
            // space), so we must account for the x/y offsets of the
            // destination surface
            j2d_glScissor(dstOps->xOffset + x1,
                          dstOps->yOffset + dstOps->height - (y1 + height),
                          width, height);
        } else {
            // complex clip; using stencil buffer
            SurfaceDataBounds span;
            RegionData clipInfo;

            J2dTraceLn(J2D_TRACE_VERBOSE, "enabling complex clip");

            Region_GetInfo(env, clip, &clipInfo);

            j2d_glDisable(GL_SCISSOR_TEST);
            j2d_glEnable(GL_STENCIL_TEST);
            j2d_glClearStencil(0x0);
            j2d_glClear(GL_STENCIL_BUFFER_BIT);
            j2d_glStencilFunc(GL_ALWAYS, 0x1, 0x1);
            j2d_glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

            // disable writes into the color buffer while we set up the clip
            j2d_glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            // use identity transform when setting the clip
            j2d_glMatrixMode(GL_MODELVIEW);
            j2d_glPushMatrix();
            j2d_glLoadIdentity();

            Region_StartIteration(env, &clipInfo);
            GLRECT_BEGIN;
            while (Region_NextIteration(&clipInfo, &span)) {
                GLRECT_BODY_XYXY(span.x1, span.y1, span.x2, span.y2);
            }
            GLRECT_END;
            Region_EndIteration(env, &clipInfo);

            // restore transform
            j2d_glPopMatrix();

            // re-enable writes into the color buffer
            j2d_glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            j2d_glStencilFunc(GL_EQUAL, 0x1, 0x1);
            j2d_glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        }
    }
}

/**
 * Initializes the OpenGL blend function state.  The modified blend rules will
 * be used if the appropriate extensions are available and the source surface
 * is not premultiplied (this will effectively treat the source surface as
 * premultiplied).
 */
static void
OGLContext_SetBlendFunc(OGLContext *oglc, jint rule, jint flags)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_SetBlendFunc");

    if ((flags & OGLC_SRC_IS_PREMULT) == 0) {
        // REMIND: we can enable a fragment shader here (where supported) to
        //         handle non-premultiplied surfaces...
        if (oglc->extInfo->blendPremult) {
            J2dTraceLn1(J2D_TRACE_VERBOSE, "separate blend funcs: %d", rule);
            j2d_glBlendFuncSeparateEXT(ModBlendRules[rule].src,
                                       ModBlendRules[rule].dst,
                                       StdBlendRules[rule].src,
                                       StdBlendRules[rule].dst);
        } else {
            // we should not hit this case because Java-level validation
            // code ensures that all sources will be premultiplied before
            // sending to OpenGL when blendPremult extension is not present
            // (but we'll just use the std rules here as a backstop)
            J2dTraceLn1(J2D_TRACE_VERBOSE, "non-premult func: %d", rule);
            j2d_glBlendFunc(StdBlendRules[rule].src, StdBlendRules[rule].dst);
        }
    } else {
        J2dTraceLn1(J2D_TRACE_VERBOSE, "standard blend func: %d", rule);
        j2d_glBlendFunc(StdBlendRules[rule].src, StdBlendRules[rule].dst);
    }
}

/**
 * Initializes the OpenGL state responsible for applying extra alpha.  This
 * step is only necessary for operations that specify OGLC_USE_EXTRA_ALPHA
 * in their context flags (e.g. any operation that uses glDrawPixels() or
 * glCopyPixels()).
 *
 * If the source is premultiplied, we need to apply the extra alpha value to
 * both alpha and color components using GL_*_SCALE.  Otherwise, we will
 * apply the extra alpha value to the alpha component only, and the
 * "blendPremult" extension will apply the alpha value to the color
 * components.
 */
static void
OGLContext_SetExtraAlpha(jfloat extraAlpha, jint flags)
{
    jfloat compAlpha = (flags & OGLC_SRC_IS_PREMULT) ? extraAlpha : 1.0f;

    J2dTraceLn2(J2D_TRACE_INFO, "in OGLContext_SetExtraAlpha (ea=%f ca=%f)",
                extraAlpha, compAlpha);

    j2d_glPixelTransferf(GL_ALPHA_SCALE, extraAlpha);
    j2d_glPixelTransferf(GL_RED_SCALE, compAlpha);
    j2d_glPixelTransferf(GL_GREEN_SCALE, compAlpha);
    j2d_glPixelTransferf(GL_BLUE_SCALE, compAlpha);
}

/**
 * Resets all values used by OGLContext_SetExtraAlpha() to their default
 * values.
 */
static void
OGLContext_ResetExtraAlpha()
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_ResetExtraAlpha");

    j2d_glPixelTransferf(GL_ALPHA_SCALE, 1.0f);
    j2d_glPixelTransferf(GL_RED_SCALE, 1.0f);
    j2d_glPixelTransferf(GL_GREEN_SCALE, 1.0f);
    j2d_glPixelTransferf(GL_BLUE_SCALE, 1.0f);
}

/**
 * Resets all OpenGL compositing state (disables blending and logic
 * operations).
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLContext_resetComposite(JNIEnv *env, jobject oc,
                                                 jlong pCtx)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_resetComposite");

    // disable blending and XOR mode
    j2d_glDisable(GL_BLEND);
    j2d_glDisable(GL_COLOR_LOGIC_OP);

    // set extra alpha to default value
    OGLContext_ResetExtraAlpha();

    // set state to default values
    oglc->compState = sun_java2d_SunGraphics2D_COMP_ISCOPY;
    oglc->extraAlpha = 1.0f;
}

/**
 * Initializes the OpenGL blending state.  XOR mode is disabled and the
 * appropriate blend functions are setup based on the AlphaComposite rule
 * constant.
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLContext_setAlphaComposite(JNIEnv *env, jobject oc,
                                                    jlong pCtx,
                                                    jint rule,
                                                    jfloat extraAlpha,
                                                    jint flags)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);

    J2dTraceLn1(J2D_TRACE_INFO,
                "in OGLContext_setAlphaComposite (flags=%d)", flags);

    // disable XOR mode
    j2d_glDisable(GL_COLOR_LOGIC_OP);

    // we can safely disable blending when:
    //   - comp is SrcNoEa or SrcOverNoEa, and
    //   - the source is opaque
    // (turning off blending can have a large positive impact on
    // performance)
    if ((rule == RULE_Src || rule == RULE_SrcOver) &&
        (extraAlpha == 1.0f) &&
        (flags & OGLC_SRC_IS_OPAQUE))
    {
        J2dTraceLn1(J2D_TRACE_VERBOSE,
                    "disabling alpha comp (rule=%d ea=1.0 src=opq)", rule);
        j2d_glDisable(GL_BLEND);
    } else {
        J2dTraceLn2(J2D_TRACE_VERBOSE,
                    "enabling alpha comp (rule=%d ea=%f)", rule, extraAlpha);
        j2d_glEnable(GL_BLEND);
        OGLContext_SetBlendFunc(oglc, rule, flags);
    }

    // setup extra alpha
    if (flags & OGLC_USE_EXTRA_ALPHA) {
        OGLContext_SetExtraAlpha(extraAlpha, flags);
    } else {
        OGLContext_ResetExtraAlpha();
    }

    // update state
    oglc->compState = sun_java2d_SunGraphics2D_COMP_ALPHA;
    oglc->extraAlpha = extraAlpha;
}

/**
 * Initializes the OpenGL logic op state to XOR mode.  Blending is disabled
 * before enabling logic op mode.  The XOR pixel value will be applied
 * later in the OGLContext_setColor() method.
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLContext_setXorComposite(JNIEnv *env, jobject oc,
                                                  jlong pCtx, jint xorPixel)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);

    J2dTraceLn1(J2D_TRACE_INFO,
                "in OGLContext_setXorComposite (xorPixel=%08x)", xorPixel);

    // disable blending mode
    j2d_glDisable(GL_BLEND);

    // enable XOR mode
    j2d_glEnable(GL_COLOR_LOGIC_OP);
    j2d_glLogicOp(GL_XOR);

    // set extra alpha to default value
    OGLContext_ResetExtraAlpha();

    // update state
    oglc->compState = sun_java2d_SunGraphics2D_COMP_XOR;
    oglc->extraAlpha = 1.0f;
}

/**
 * Initializes the OpenGL transform state.  If the xform parameter is null,
 * the modelview transform (and pixel transform, if the extension is available)
 * is set to the identity matrix.  Otherwise, update the modelview transform
 * (and pixel transform, if necessary) using the values from the xform object.
 *
 * REMIND: it may be worthwhile to add serial id to AffineTransform, so we
 *         could do a quick check to see if the xform has changed since
 *         last time... a simple object compare won't suffice...
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLContext_setTransform(JNIEnv *env, jobject oc,
                                               jlong pCtx,
                                               jobject xform,
                                               jdouble m00, jdouble m10,
                                               jdouble m01, jdouble m11,
                                               jdouble m02, jdouble m12)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_setTransform");

    if (xform == NULL) {
        J2dTraceLn(J2D_TRACE_VERBOSE, "disabling transform");

        j2d_glMatrixMode(GL_MODELVIEW);
        j2d_glLoadIdentity();        
    } else {
        J2dTraceLn(J2D_TRACE_VERBOSE, "enabling transform");

        if (oglc->xformMatrix == NULL) {
            size_t arrsize = 16 * sizeof(GLdouble);
            oglc->xformMatrix = (GLdouble *)malloc(arrsize);
            memset(oglc->xformMatrix, 0, arrsize);
            oglc->xformMatrix[10] = 1.0;
            oglc->xformMatrix[15] = 1.0;
        }

        // copy values from AffineTransform object into native matrix array
        oglc->xformMatrix[0] = m00;
        oglc->xformMatrix[1] = m10;
        oglc->xformMatrix[4] = m01;
        oglc->xformMatrix[5] = m11;
        oglc->xformMatrix[12] = m02;
        oglc->xformMatrix[13] = m12;

        J2dTraceLn3(J2D_TRACE_VERBOSE, "[%lf %lf %lf]",
                    oglc->xformMatrix[0], oglc->xformMatrix[4],
                    oglc->xformMatrix[12]);
        J2dTraceLn3(J2D_TRACE_VERBOSE, "[%lf %lf %lf]",
                    oglc->xformMatrix[1], oglc->xformMatrix[5],
                    oglc->xformMatrix[13]);

        j2d_glMatrixMode(GL_MODELVIEW);
        j2d_glLoadMatrixd(oglc->xformMatrix);
    }
}

/**
 * Initializes the OpenGL color state.  The extra alpha value may be
 * conditionally applied if the current composite is an AlphaComposite.
 * Likewise, the XOR pixel value may be applied if the current composite is
 * an XORComposite.
 *
 * REMIND: extra alpha case may be off by 1 (may need to add 0.5)...
 */
JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLContext_setColor(JNIEnv *env, jobject oc,
                                           jlong pCtx,
                                           jint pixel, jint flags)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);

    J2dTraceLn2(J2D_TRACE_INFO,
                "in OGLContext_setColor (pixel=%08x flags=%d)",
                pixel, flags);

    if (oglc->compState != sun_java2d_SunGraphics2D_COMP_XOR) {
        jfloat ea = oglc->extraAlpha;
        GLubyte a = (GLubyte)(pixel >> 24);

        if (ea == 1.0f) {
            J2dTraceLn1(J2D_TRACE_VERBOSE,
                        "updating color (argb=%08x ea=1.0)", pixel);

            j2d_glColor4ub((GLubyte)(pixel >> 16),
                           (GLubyte)(pixel >>  8),
                           (GLubyte)(pixel >>  0), a);
        } else {
            if ((flags & OGLC_SRC_IS_PREMULT) == 0) {
                // only apply extra alpha to alpha value; ea will be
                // applied to components later as an effect of the
                // blending process
                GLfloat fa = (a / 255.0f) * ea;
                GLfloat r = ((GLubyte)(pixel >> 16)) / 255.0f;
                GLfloat g = ((GLubyte)(pixel >>  8)) / 255.0f;
                GLfloat b = ((GLubyte)(pixel >>  0)) / 255.0f;

                J2dTraceLn5(J2D_TRACE_VERBOSE,
                            "updating color (r=%f g=%f b=%f a=%f ea=%f)",
                            r, g, b, fa, ea);

                j2d_glColor4f(r, g, b, fa);
            } else {
                // apply extra alpha to both alpha and component values
                GLfloat fa = (a / 255.0f) * ea;
                GLfloat r = (((GLubyte)(pixel >> 16)) / 255.0f) * ea;
                GLfloat g = (((GLubyte)(pixel >>  8)) / 255.0f) * ea;
                GLfloat b = (((GLubyte)(pixel >>  0)) / 255.0f) * ea;

                J2dTraceLn5(J2D_TRACE_VERBOSE,
                            "updating color (r=%f g=%f b=%f a=%f ea=%f)",
                            r, g, b, fa, ea);

                j2d_glColor4f(r, g, b, fa);
            }
        }
    } else {
        pixel ^= oglc->xorPixel;

        J2dTraceLn4(J2D_TRACE_VERBOSE,
                    "updating xor color (r=%02x g=%02x b=%02x xorpixel=%08x)",
                    pixel >> 16, pixel >> 8, pixel, oglc->xorPixel);

        j2d_glColor3ub((GLubyte)(pixel >> 16),
                       (GLubyte)(pixel >>  8),
                       (GLubyte)(pixel >>  0));
    }
}

/**
 * Convenience method that invokes the Java-level
 * OGLContext.getContext() method using the given OGLSurfaceData object
 * reference.  Returns a pointer to the native OGLContext structure.
 */
OGLContext *
OGLContext_GetContext(JNIEnv *env, jobject dstData)
{
    jobject sdObject;
    jlong oglc = 0L;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_GetContext");

    sdObject = (*env)->NewLocalRef(env, dstData);
    if (sdObject != NULL) {
        oglc = (*env)->CallStaticLongMethod(env, oglcClass, getContextID,
                                            sdObject);
        (*env)->DeleteLocalRef(env, sdObject);
    }

    return (OGLContext *)jlong_to_ptr(oglc);
}

/**
 * Convenience method that invokes the Java-level
 * OGLContext.invalidateCurrentContext() method.
 */
void
OGLContext_InvalidateCurrentContext(JNIEnv *env)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_InvalidateCurrentContext");

    (*env)->CallStaticVoidMethod(env, oglcClass, invalidateID);
}

/**
 * Causes an asynchronous flush event to be posted to the Java EventQueue,
 * which will eventually flush the pipeline after pending events are consumed.
 */
void
OGLContext_InvokeGLFlush(JNIEnv *env)
{
    (*env)->CallStaticVoidMethod(env, oglcClass, invokeFlushID);
}

void
OGLContext_Flush(JNIEnv *env, OGLContext *oglc)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_Flush");

    if (oglc->onJED) {
        // if we're on the JED thread, we can use the native Toolkit thread
        // flushing mechanism, which will post an asynchronous event that will
        // eventually flush the context that is current to this thread
        needGLFlush = JNI_TRUE;
        OGLSD_OutputFlush(env);
    } else {
        // otherwise, we have no choice but to flush immediately (or else
        // any rendering not invoked from the JED thread could be left
        // unflushed)
        j2d_glFlush();
    }
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLContext_flushPipeline(JNIEnv *env,
                                                jclass oglsd)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_flushPipeline");

    // invoke the pipeline flush (we assume that this method will only
    // be called from the Java EventDispatchThread, so this will flush
    // the context associated with that thread)
    j2d_glFlush();
}

/**
 * Returns JNI_TRUE if the given extension name is available for the current
 * GraphicsConfig; JNI_FALSE otherwise.  An extension is considered available
 * if its identifier string is found amongst the space-delimited GL_EXTENSIONS
 * string.
 *
 * Adapted from the OpenGL Red Book, pg. 506.
 */
jboolean
OGLContext_IsExtensionAvailable(const char *extString, char *extName)
{
    jboolean ret = JNI_FALSE;
    char *p = (char *)extString;
    char *end;

    if (extString == NULL) {
        J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_IsExtensionAvailable");
        J2dTraceLn(J2D_TRACE_ERROR, "extension string is null");
        return JNI_FALSE;
    }

    end = p + strlen(p);

    while (p < end) {
        size_t n = strcspn(p, " ");

        if ((strlen(extName) == n) && (strncmp(extName, p, n) == 0)) {
            ret = JNI_TRUE;
            break;
        }

        p += (n + 1);
    }

    J2dTraceLn2(J2D_TRACE_INFO, "in OGLContext_IsExtensionAvailable (%s=%s)",
                extName, ret ? "true" : "false");

    return ret;
}

/**
 * Checks for the presence of the required and optional extensions used by
 * the Java 2D OpenGL pipeline.  The given OGLExtInfo structure is updated
 * with boolean values reflecting the availability of these extensions.
 */
void
OGLContext_GetExtensionInfo(OGLExtInfo *extInfo)
{
    const char *e = (char *)j2d_glGetString(GL_EXTENSIONS);

    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_GetExtensionInfo");

    extInfo->imaging =
        OGLContext_IsExtensionAvailable(e, "GL_ARB_imaging");
    extInfo->blendPremult =
        OGLContext_IsExtensionAvailable(e, "GL_EXT_blend_func_separate") &&
        OGLContext_IsExtensionAvailable(e, "GL_SUN_blend_src_mult_dst_alpha");
    extInfo->multitexture =
        OGLContext_IsExtensionAvailable(e, "GL_ARB_multitexture");
    extInfo->texNonPow2 =
        OGLContext_IsExtensionAvailable(e, "GL_ARB_texture_non_power_of_two");
}

/**
 * Returns JNI_TRUE if the given GL_VERSION string meets the minimum
 * requirements (>= 1.2); JNI_FALSE otherwise.
 */
jboolean
OGLContext_IsVersionSupported(const unsigned char *versionstr)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLContext_IsVersionSupported");

    // note that this check allows for OpenGL 2.x
    return ((versionstr[0] == '1' && versionstr[2] >= '2') ||
            (versionstr[0] >= '2'));
}

#endif /* !HEADLESS */
