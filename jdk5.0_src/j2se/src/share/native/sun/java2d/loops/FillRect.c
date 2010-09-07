/*
 * @(#)FillRect.c	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GraphicsPrimitiveMgr.h"

#include "sun_java2d_loops_FillRect.h"

/*
 * Class:     sun_java2d_loops_FillRect
 * Method:    FillRect
 * Signature: (Lsun/java2d/SunGraphics2D;Lsun/java2d/SurfaceData;IIII)V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_loops_FillRect_FillRect
    (JNIEnv *env, jobject self,
     jobject sg2d, jobject sData,
     jint x, jint y, jint w, jint h)
{
    SurfaceDataOps *sdOps;
    SurfaceDataRasInfo rasInfo;
    NativePrimitive *pPrim;
    CompositeInfo compInfo;
    jint pixel = GrPrim_Sg2dGetPixel(env, sg2d);

    if (w <= 0 || h <= 0) {
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

    GrPrim_Sg2dGetClip(env, sg2d, &rasInfo.bounds);
    SurfaceData_IntersectBoundsXYWH(&rasInfo.bounds, x, y, w, h);
    if (rasInfo.bounds.y2 <= rasInfo.bounds.y1 ||
	rasInfo.bounds.x2 <= rasInfo.bounds.x1)
    {
	return;
    }

    if (sdOps->Lock(env, sdOps, &rasInfo, pPrim->dstflags) != SD_SUCCESS) {
	return;
    }

    if (rasInfo.bounds.x2 > rasInfo.bounds.x1 &&
	rasInfo.bounds.y2 > rasInfo.bounds.y1)
    {
	sdOps->GetRasInfo(env, sdOps, &rasInfo);
	if (rasInfo.rasBase) {
	    (*pPrim->funcs.fillrect)(&rasInfo,
				     rasInfo.bounds.x1, rasInfo.bounds.y1,
				     rasInfo.bounds.x2, rasInfo.bounds.y2,
				     pixel, pPrim, &compInfo);
	}
	SurfaceData_InvokeRelease(env, sdOps, &rasInfo);
    }
    SurfaceData_InvokeUnlock(env, sdOps, &rasInfo);
}
