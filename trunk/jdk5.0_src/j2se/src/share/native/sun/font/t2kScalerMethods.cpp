/*
 * @(#)t2kScalerMethods.cpp	1.4 12/19/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sunt2kscaler.h"
#include "sunfontids.h"
#include "GeneralPath.h"
#include <stdio.h>

extern "C" {

static void getGlyphGeneralPath(JNIEnv* env, jobject font2D, jlong pScalerContext, 
                                jint glyphCode, jfloat xpos, jfloat ypos,
                                GeneralPath* gp) {

    if (glyphCode >= INVISIBLE_GLYPHS) {
      return;
    }

    T2KScalerContext *context = (T2KScalerContext*)pScalerContext;
    T2KScalerInfo *scalerInfo = context->scalerInfo;
    T2K *t2k = scalerInfo->t2k;
    UInt8 renderFlags = ((context->t2kFlags | T2K_RETURN_OUTLINES)
			 & ~T2K_GRID_FIT);
    jboolean isQuadPath = scalerInfo->pathType == QUADPATHTYPE;

    int errCode;

    if (scalerInfo == theNullScaler || context == theNullScalerContext) {
	return;
    }
    errCode = setupT2KContext(env, font2D, scalerInfo, context, false);
    if (errCode) {
      return;
    }

    T2K_RenderGlyph(t2k, glyphCode, 0, 0, context->greyLevel, renderFlags,
		    &errCode);
    t2kIfDebugMessage(errCode, "T2K_RenderGlyph failed", errCode);

    addGlyphToGeneralPath(*t2k->glyph, *gp, xpos, ypos, isQuadPath);

    T2K_PurgeMemory(t2k, 1, &errCode);
    t2kIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
}

JNIEXPORT jobject JNICALL
Java_sun_font_FileFont_getGlyphOutline
    (JNIEnv *env, jobject font2D, jlong pScalerContext,
    jint glyphCode, jfloat xpos, jfloat ypos) {

    GeneralPath gp;
    getGlyphGeneralPath(env, font2D, pScalerContext, glyphCode, xpos, ypos, &gp);
    return gp.getShape(env);
}

JNIEXPORT jobject JNICALL
Java_sun_font_FileFont_getGlyphOutlineBounds
    (JNIEnv *env, jobject font2D, jlong pScalerContext,	int glyphCode) {

    GeneralPath gp;
    getGlyphGeneralPath(env, font2D, pScalerContext, glyphCode, 0.0f, 0.0f, &gp);
    return gp.getBounds(env);
}

JNIEXPORT jobject JNICALL
Java_sun_font_FileFont_getGlyphVectorOutline
    (JNIEnv *env, jobject font2D, jintArray glyphArray, int numGlyphs,
    jlong pScalerContext, jfloat xpos, jfloat ypos) {

    int i;
    T2KScalerContext *context = (T2KScalerContext*)pScalerContext;
    T2KScalerInfo *scalerInfo = context->scalerInfo;
    T2K *t2k = scalerInfo->t2k;
    GeneralPath generalPath;

    if (scalerInfo == theNullScaler || context == theNullScalerContext) {
	return generalPath.getShape(env);
    }

    UInt8 renderFlags = ((context->t2kFlags | T2K_RETURN_OUTLINES)
			 & ~T2K_GRID_FIT);
    jint *glyphs = (jint*)malloc(sizeof(jint)*numGlyphs);
    jboolean isQuadPath = scalerInfo->pathType == QUADPATHTYPE;

    int errCode;

    errCode = setupT2KContext(env, font2D, scalerInfo, context, false);
    if (errCode) {
	return generalPath.getShape(env);
    }

    env->GetIntArrayRegion(glyphArray, 0, numGlyphs, glyphs);
    for (i=0; i<numGlyphs;i++) {
	if (glyphs[i] >= INVISIBLE_GLYPHS) {
	    continue;
	}
	T2K_RenderGlyph(t2k, glyphs[i], 0, 0, context->greyLevel, renderFlags,
			&errCode);
	t2kIfDebugMessage(errCode, "T2K_RenderGlyph failed", errCode);

	addGlyphToGeneralPath(*t2k->glyph, generalPath,
			      xpos, ypos, isQuadPath);

	T2K_PurgeMemory(t2k, 1, &errCode);
	t2kIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
    }

    free(glyphs);
    return generalPath.getShape(env);
}

} /* End extern "C" */
