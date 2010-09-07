/*
 * @(#)MaskFill.c	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GraphicsPrimitiveMgr.h"

#include "sun_java2d_loops_MaskFill.h"

/*
 * Class:     sun_java2d_loops_MaskFill
 * Method:    MaskFill
 * Signature: (Lsun/java2d/SunGraphics2D;Lsun/java2d/SurfaceData;Ljava/awt/Composite;IIII[BII)V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_loops_MaskFill_MaskFill
    (JNIEnv *env, jobject self,
     jobject sg2d, jobject sData, jobject comp,
     jint x, jint y, jint w, jint h,
     jbyteArray maskArray, jint maskoff, jint maskscan)
{
    SurfaceDataOps *sdOps;
    SurfaceDataRasInfo rasInfo;
    NativePrimitive *pPrim;
    CompositeInfo compInfo;

    pPrim = GetNativePrim(env, self);
    if (pPrim == NULL) {
	return;
    }
    if (pPrim->pCompType->getCompInfo != NULL) {
	(*pPrim->pCompType->getCompInfo)(env, &compInfo, comp);
    }

    sdOps = SurfaceData_GetOps(env, sData);
    if (sdOps == 0) {
	return;
    }

    rasInfo.bounds.x1 = x;
    rasInfo.bounds.y1 = y;
    rasInfo.bounds.x2 = x + w;
    rasInfo.bounds.y2 = y + h;
    if (sdOps->Lock(env, sdOps, &rasInfo, pPrim->dstflags) != SD_SUCCESS) {
	return;
    }

    if (rasInfo.bounds.x2 > rasInfo.bounds.x1 &&
	rasInfo.bounds.y2 > rasInfo.bounds.y1)
    {
	jint color = GrPrim_Sg2dGetRGB(env, sg2d);
	sdOps->GetRasInfo(env, sdOps, &rasInfo);
	if (rasInfo.rasBase) {
	    jint width = rasInfo.bounds.x2 - rasInfo.bounds.x1;
	    jint height = rasInfo.bounds.y2 - rasInfo.bounds.y1;
	    void *pDst = PtrCoord(rasInfo.rasBase,
				  rasInfo.bounds.x1, rasInfo.pixelStride,
				  rasInfo.bounds.y1, rasInfo.scanStride);
	    unsigned char *pMask =
		(maskArray
		 ? (*env)->GetPrimitiveArrayCritical(env, maskArray, 0)
		 : 0);
            maskoff += ((rasInfo.bounds.y1 - y) * maskscan +
			(rasInfo.bounds.x1 - x));
	    (*pPrim->funcs.maskfill)(pDst,
				     pMask, maskoff, maskscan,
				     width, height,
				     color, &rasInfo,
				     pPrim, &compInfo);
	    if (pMask) {
		(*env)->ReleasePrimitiveArrayCritical(env, maskArray,
						      pMask, JNI_ABORT);
	    }
	}
	SurfaceData_InvokeRelease(env, sdOps, &rasInfo);
    }
    SurfaceData_InvokeUnlock(env, sdOps, &rasInfo);
}
