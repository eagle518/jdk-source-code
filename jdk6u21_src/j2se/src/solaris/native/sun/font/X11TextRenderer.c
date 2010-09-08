/*
 * @(#)X11TextRenderer.c	1.14 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Important note : All AWTxxx functions are defined in font.h.
 * These were added to remove the dependency of this file on X11.
 * These functions are used to perform X11 operations and should
 * be "stubbed out" in environments that do not support X11.
 * The implementation of these functions has been moved from this file
 * into X11TextRenderer_md.c, which is compiled into another library.
 */

#include "sun_font_X11TextRenderer.h"

#include "Region.h"
#include "SurfaceData.h"
#include "GraphicsPrimitiveMgr.h"
#include "glyphblitting.h"
#include "sunfontids.h"
#include <malloc.h>


JNIEXPORT void JNICALL AWTDrawGlyphList
(JNIEnv *env, jobject xtr,
 jlong dstData, jlong gc,
 SurfaceDataBounds *bounds, ImageRef *glyphs, jint totalGlyphs);

/*
 * Class:     sun_font_X11TextRenderer
 * Method:    doDrawGlyphList
 * Signature: (Lsun/java2d/SurfaceData;Ljava/awt/Rectangle;ILsun/font/GlyphList;J)V
 */
JNIEXPORT void JNICALL Java_sun_font_X11TextRenderer_doDrawGlyphList
    (JNIEnv *env, jobject xtr,
     jlong dstData, jlong xgc, jobject clip,
     jobject glyphlist)
{
    GlyphBlitVector* gbv;
    SurfaceDataBounds bounds;
    Region_GetBounds(env, clip, &bounds);

    if ((gbv = setupBlitVector(env, glyphlist)) == NULL) {
	return;
    }
    if (!RefineBounds(gbv, &bounds)) {
        free(gbv);
        return;
    }
    AWTDrawGlyphList(env, xtr, dstData, xgc,
		     &bounds, gbv->glyphs, gbv->numGlyphs);
    free(gbv);
}
