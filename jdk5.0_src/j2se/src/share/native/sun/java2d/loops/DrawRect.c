/*
 * @(#)DrawRect.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GraphicsPrimitiveMgr.h"
#include "LineUtils.h"

#include "sun_java2d_loops_DrawRect.h"

/*
 * Class:     sun_java2d_loops_DrawRect
 * Method:    DrawRect
 * Signature: (Lsun/java2d/SunGraphics2D;Lsun/java2d/SurfaceData;IIII)V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_loops_DrawRect_DrawRect
    (JNIEnv *env, jobject self,
     jobject sg2d, jobject sData,
     jint x, jint y, jint w, jint h)
{
    SurfaceDataOps *sdOps;
    SurfaceDataRasInfo rasInfo;
    NativePrimitive *pPrim;
    CompositeInfo compInfo;
    jint lox, loy, hix, hiy;
    jint pixel = GrPrim_Sg2dGetPixel(env, sg2d);

    if (w < 0 || h < 0) {
	return;
    }

    pPrim = GetNativePrim(env, self);
    if (pPrim == NULL) {
	return;
    }
    if (pPrim->pCompType->getCompInfo != NULL) { 
        GrPrim_Sg2dGetCompInfo(env, sg2d, pPrim, &compInfo); 
    }

    sdOps = SurfaceData_GetOps(env, sData);
    if (sdOps == 0) {
	return;
    }

    lox = x;
    loy = y;
    hix = x + w + 1;
    hiy = y + h + 1;
    if (hix < lox) {
	hix = 0x7fffffff;
    }
    if (hiy < loy) {
	hiy = 0x7fffffff;
    }

    GrPrim_Sg2dGetClip(env, sg2d, &rasInfo.bounds);
    if (rasInfo.bounds.x1 < lox) rasInfo.bounds.x1 = lox;
    if (rasInfo.bounds.y1 < loy) rasInfo.bounds.y1 = loy;
    if (rasInfo.bounds.x2 > hix) rasInfo.bounds.x2 = hix;
    if (rasInfo.bounds.y2 > hiy) rasInfo.bounds.y2 = hiy;
    if (sdOps->Lock(env, sdOps, &rasInfo, pPrim->dstflags) != SD_SUCCESS) {
	return;
    }

    if (rasInfo.bounds.x2 > rasInfo.bounds.x1 &&
	rasInfo.bounds.y2 > rasInfo.bounds.y1)
    {
	sdOps->GetRasInfo(env, sdOps, &rasInfo);
	if (rasInfo.rasBase) {
	    DrawLineFunc *pLine = pPrim->funcs.drawline;
	    int loyin = (loy == rasInfo.bounds.y1);
	    int hiyin = (hiy == rasInfo.bounds.y2);
	    int xsize = (rasInfo.bounds.x2 - rasInfo.bounds.x1);
	    int ysize = (rasInfo.bounds.y2 - rasInfo.bounds.y1 - loyin - hiyin);
	    /*
	     * To avoid drawing the corners twice (both for performance
	     * and because XOR erases them otherwise) and to maximize the
	     * number of pixels we draw in the horizontal portions
	     * which are more cache-friendly, we include the corner
	     * pixels only in the top and bottom segments.
	     * We also protect against degenerate rectangles where we
	     * would draw the same line for top & bottom or left & right.
	     */
	    if (loyin) {
		/* Line across the top */
		(*pLine)(&rasInfo,
			 rasInfo.bounds.x1, rasInfo.bounds.y1,
			 pixel, xsize, 0,
			 BUMP_POS_PIXEL, 0, BUMP_NOOP, 0, pPrim, &compInfo);
	    }
	    if (lox == rasInfo.bounds.x1 && ysize > 0) {
		/* Line down the left side */
		(*pLine)(&rasInfo,
			 rasInfo.bounds.x1, rasInfo.bounds.y1 + loyin,
			 pixel, ysize, 0,
			 BUMP_POS_SCAN, 0, BUMP_NOOP, 0, pPrim, &compInfo);
	    }
	    if (hix == rasInfo.bounds.x2 && ysize > 0 && lox != hix - 1) {
		/* Line down the right side */
		(*pLine)(&rasInfo,
			 rasInfo.bounds.x2 - 1, rasInfo.bounds.y1 + loyin,
			 pixel, ysize, 0,
			 BUMP_POS_SCAN, 0, BUMP_NOOP, 0, pPrim, &compInfo);
	    }
	    if (hiyin && loy != hiy - 1) {
		/* Line across the bottom */
		(*pLine)(&rasInfo,
			 rasInfo.bounds.x1, rasInfo.bounds.y2 - 1,
			 pixel, xsize, 0,
			 BUMP_POS_PIXEL, 0, BUMP_NOOP, 0, pPrim, &compInfo);
	    }
	}
	SurfaceData_InvokeRelease(env, sdOps, &rasInfo);
    }
    SurfaceData_InvokeUnlock(env, sdOps, &rasInfo);
}
