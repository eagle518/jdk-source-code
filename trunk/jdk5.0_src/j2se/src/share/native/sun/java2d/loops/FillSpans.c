/*
 * @(#)FillSpans.c	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni_util.h"
#include "jlong.h"

#include "sun_java2d_loops_FillSpans.h"

#include "GraphicsPrimitiveMgr.h"

/*
 * Class:     sun_java2d_loops_FillSpans
 * Method:    FillSpans
 * Signature: (Lsun/java2d/SunGraphics2D;Lsun/java2d/SurfaceData;Lsun/java2d/pipe/SpanIterator;)V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_loops_FillSpans_FillSpans
    (JNIEnv *env, jobject self,
     jobject sg2d, jobject sData, jint pixel, jlong pIterator, jobject si)
{
    SpanIteratorFuncs *pSpanFuncs;
    SurfaceDataOps *sdOps;
    SurfaceDataRasInfo rasInfo;
    void *siData;
    jint bbox[4];
    NativePrimitive *pPrim;
    CompositeInfo compInfo;

    pSpanFuncs = (SpanIteratorFuncs *) jlong_to_ptr(pIterator);
    if (pSpanFuncs == NULL) {
	JNU_ThrowNullPointerException(env, "native iterator not supplied");
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
    if (sdOps == NULL) {
	return;
    }

    siData = (*pSpanFuncs->open)(env, si);

    (*pSpanFuncs->getPathBox)(env, siData, bbox);
    rasInfo.bounds.x1 = bbox[0];
    rasInfo.bounds.y1 = bbox[1];
    rasInfo.bounds.x2 = bbox[2];
    rasInfo.bounds.y2 = bbox[3];

    if (sdOps->Lock(env, sdOps, &rasInfo, pPrim->dstflags) != SD_SUCCESS) {
	/* Lock threw an exception */
	(*pSpanFuncs->close)(env, siData);
	return;
    }
    (*pSpanFuncs->intersectClipBox)(env, siData,
				    rasInfo.bounds.x1,
				    rasInfo.bounds.y1,
				    rasInfo.bounds.x2,
				    rasInfo.bounds.y2);

    sdOps->GetRasInfo(env, sdOps, &rasInfo);
    /* Protect against silent failure of GetRasInfo */
    if (rasInfo.rasBase != NULL) {
	pPrim->funcs.fillspans(&rasInfo, pSpanFuncs, siData, 
                               pixel, pPrim, &compInfo);
    }

    SurfaceData_InvokeRelease(env, sdOps, &rasInfo);
    (*pSpanFuncs->close)(env, siData);
    SurfaceData_InvokeUnlock(env, sdOps, &rasInfo);
}
