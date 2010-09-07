/*
 * @(#)DrawGlyphList.c	1.11 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jlong.h"
#include "sunt2kscaler.h"
#include "glyphblitting.h"
#include "sunfontids.h"
#include "GraphicsPrimitiveMgr.h"
#include "sun_java2d_loops_DrawGlyphList.h"
#include "sun_java2d_loops_DrawGlyphListAA.h"


GlyphBlitVector* setupBlitVector(JNIEnv *env, jobject glyphlist) {

    int g, bytesNeeded;
    jlong *imagePtrs;
    jfloat* positions = NULL;
    GlyphInfo *ginfo;
    GlyphBlitVector *gbv;

    jfloat x = (*env)->GetFloatField(env, glyphlist, sunFontIDs.glyphListX);
    jfloat y = (*env)->GetFloatField(env, glyphlist, sunFontIDs.glyphListY);
    jint len =  (*env)->GetIntField(env, glyphlist, sunFontIDs.glyphListLen);
    jlongArray glyphImages = (jlongArray)
	(*env)->GetObjectField(env, glyphlist, sunFontIDs.glyphImages);
    jfloatArray glyphPositions =
      (*env)->GetBooleanField(env, glyphlist, sunFontIDs.glyphListUsePos) 
        ? (jfloatArray)
      (*env)->GetObjectField(env, glyphlist, sunFontIDs.glyphListPos)
        : NULL;
        
    bytesNeeded = sizeof(GlyphBlitVector)+sizeof(ImageRef)*len;
    gbv = (GlyphBlitVector*)malloc(bytesNeeded);
    gbv->numGlyphs = len;
    gbv->glyphs = (ImageRef*)((unsigned char*)gbv+sizeof(GlyphBlitVector));

    imagePtrs = (*env)->GetPrimitiveArrayCritical(env, glyphImages, NULL);
    if (imagePtrs == NULL) {
        free(gbv);
	return (GlyphBlitVector*)NULL;
    }

    if (glyphPositions) {
	int n = -1;

        positions =
	  (*env)->GetPrimitiveArrayCritical(env, glyphPositions, NULL);
        if (positions == NULL) {
            (*env)->ReleasePrimitiveArrayCritical(env, glyphImages,
						  imagePtrs, JNI_ABORT);
	    free(gbv);
            return (GlyphBlitVector*)NULL;
        }

	for (g=0; g<len; g++) {
	    jfloat px = x + positions[++n];
	    jfloat py = y + positions[++n];

	    ginfo = (GlyphInfo*)imagePtrs[g];
            gbv->glyphs[g].glyphInfo = ginfo;
	    gbv->glyphs[g].pixels = ginfo->image;
	    gbv->glyphs[g].width = ginfo->width;
	    gbv->glyphs[g].height = ginfo->height;
	    gbv->glyphs[g].x = (int)(px+ginfo->topLeftX);
	    gbv->glyphs[g].y = (int)(py+ginfo->topLeftY);
	}
	(*env)->ReleasePrimitiveArrayCritical(env,glyphPositions,
					      positions, JNI_ABORT);
    } else {
        for (g=0; g<len; g++) {
            ginfo = (GlyphInfo*)imagePtrs[g];
            gbv->glyphs[g].glyphInfo = ginfo;
            gbv->glyphs[g].pixels = ginfo->image;
            gbv->glyphs[g].width = ginfo->width;
            gbv->glyphs[g].height = ginfo->height;
	    gbv->glyphs[g].x = (int)(x+ginfo->topLeftX);
	    gbv->glyphs[g].y = (int)(y+ginfo->topLeftY);

	    /* copy image data into this array at x/y locations */
            x += ginfo->advanceX;
            y += ginfo->advanceY;
        }
    }
    
    (*env)->ReleasePrimitiveArrayCritical(env, glyphImages, imagePtrs,
					  JNI_ABORT);
    return gbv;
}

jint RefineBounds(GlyphBlitVector *gbv, SurfaceDataBounds *bounds) {
    int index;
    jint dx1, dy1, dx2, dy2;
    ImageRef glyphImage;
    int num = gbv->numGlyphs;
    SurfaceDataBounds glyphs;

    glyphs.x1 = glyphs.y1 = 0x7fffffff;
    glyphs.x2 = glyphs.y2 = 0x80000000;
    for (index = 0; index < num; index++) {
        glyphImage = gbv->glyphs[index];
        dx1 = (jint) glyphImage.x;
        dy1 = (jint) glyphImage.y;
        dx2 = dx1 + glyphImage.width;
        dy2 = dy1 + glyphImage.height;
	if (glyphs.x1 > dx1) glyphs.x1 = dx1;
	if (glyphs.y1 > dy1) glyphs.y1 = dy1;
	if (glyphs.x2 < dx2) glyphs.x2 = dx2;
	if (glyphs.y2 < dy2) glyphs.y2 = dy2;
    }

    SurfaceData_IntersectBounds(bounds, &glyphs);
    return (bounds->x1 < bounds->x2 && bounds->y1 < bounds->y2);
}




/* since the AA and non-AA loop functions share a common method
 * signature, can call both through this common function since
 * there's no difference except for the inner loop.
 * This could be a macro but there's enough of those already.
 */
static void drawGlyphList(JNIEnv *env, jobject self,
			  jobject sg2d, jobject sData,
			  GlyphBlitVector *gbv, jint pixel, jint color,
			  NativePrimitive *pPrim, DrawGlyphListFunc *func) {

    SurfaceDataOps *sdOps;
    SurfaceDataRasInfo rasInfo;
    CompositeInfo compInfo;
    int clipLeft, clipRight, clipTop, clipBottom;
    int ret;

    sdOps = SurfaceData_GetOps(env, sData);
    if (sdOps == 0) {
	return;
    }

    if (pPrim->pCompType->getCompInfo != NULL) {
        GrPrim_Sg2dGetCompInfo(env, sg2d, pPrim, &compInfo);
    }

    GrPrim_Sg2dGetClip(env, sg2d, &rasInfo.bounds);
    if (rasInfo.bounds.y2 <= rasInfo.bounds.y1 ||
	rasInfo.bounds.x2 <= rasInfo.bounds.x1)
    {
	return;
    }

    ret = sdOps->Lock(env, sdOps, &rasInfo, pPrim->dstflags);
    if (ret != SD_SUCCESS) { 
	if (ret == SD_SLOWLOCK) {
	    if (!RefineBounds(gbv, &rasInfo.bounds)) {
		SurfaceData_InvokeUnlock(env, sdOps, &rasInfo);
		return;
	    }
	} else {
	    return;
	}
    }

    sdOps->GetRasInfo(env, sdOps, &rasInfo);
    if (!rasInfo.rasBase) { 
	SurfaceData_InvokeUnlock(env, sdOps, &rasInfo);
	return;
    }
    clipLeft    = rasInfo.bounds.x1;
    clipRight   = rasInfo.bounds.x2;
    clipTop     = rasInfo.bounds.y1;
    clipBottom  = rasInfo.bounds.y2;
    if (clipRight > clipLeft &&	clipBottom > clipTop) {

	(*func)(&rasInfo,
		gbv->glyphs, gbv->numGlyphs,
                pixel, color,
		clipLeft, clipTop,
		clipRight, clipBottom,
                pPrim, &compInfo);              
	SurfaceData_InvokeRelease(env, sdOps, &rasInfo);

    }
    SurfaceData_InvokeUnlock(env, sdOps, &rasInfo);
}

/*
 * Class:     sun_java2d_loops_DrawGlyphList
 * Method:    DrawGlyphList
 * Signature: (Lsun/java2d/SunGraphics2D;Lsun/java2d/SurfaceData;Lsun/java2d/font/GlyphList;J)V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_loops_DrawGlyphList_DrawGlyphList
    (JNIEnv *env, jobject self,
     jobject sg2d, jobject sData, jobject glyphlist) {

    jint pixel, color;
    GlyphBlitVector* gbv;
    NativePrimitive *pPrim;

    if ((pPrim = GetNativePrim(env, self)) == NULL) {
	return;
    }

    if ((gbv = setupBlitVector(env, glyphlist)) == NULL) {
	return;
    }

    pixel = GrPrim_Sg2dGetPixel(env, sg2d);
    color = GrPrim_Sg2dGetRGB(env, sg2d);
    drawGlyphList(env, self, sg2d, sData, gbv, pixel, color,
		  pPrim, pPrim->funcs.drawglyphlist);
    free(gbv);

}

/*
 * Class:     sun_java2d_loops_DrawGlyphListAA
 * Method:    DrawGlyphListAA
 * Signature: (Lsun/java2d/SunGraphics2D;Lsun/java2d/SurfaceData;Lsun/java2d/font/GlyphList;J)V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_loops_DrawGlyphListAA_DrawGlyphListAA
    (JNIEnv *env, jobject self,
     jobject sg2d, jobject sData, jobject glyphlist) {

    jint pixel, color;
    GlyphBlitVector* gbv;
    NativePrimitive *pPrim;

    if ((pPrim = GetNativePrim(env, self)) == NULL) {
	return;
    }

    if ((gbv = setupBlitVector(env, glyphlist)) == NULL) {
	return;
    }
    pixel = GrPrim_Sg2dGetPixel(env, sg2d);
    color = GrPrim_Sg2dGetRGB(env, sg2d);
    drawGlyphList(env, self, sg2d, sData, gbv, pixel, color,
		  pPrim, pPrim->funcs.drawglyphlistaa);
    free(gbv);
}

