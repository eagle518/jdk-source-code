/*
 * @(#)ScaledBlit.c	1.7 04/01/14
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni_util.h"
#include "GraphicsPrimitiveMgr.h"
#include "Region.h"

#include "sun_java2d_loops_ScaledBlit.h"

static void
Scale_adjustUp(jint srcPos, jint srcLimit,
	       jint *pSrcLoc, jint srcInc, jint shift,
	       jint *pDstPos)
{
    jint srcLoc = *pSrcLoc;
    jint nsteps;

    if (srcPos >= srcLimit) return;
    /*
     * Overflow checking:
     * (srcLimit - srcPos) must be less than srcw
     * (srcw << shift) does not overflow so
     * ((srcLimit - srcPos) << shift) must not overflow either
     * From the calculation of srcLoc we have (0 <= srcLoc < srcInc)
     * and srcInc is less than "shift" bits wide
     * therefore (- srcLoc + srcInc - 1) must be less than "shift"
     * bits of precision.
     * Thus the sum below should not overflow an integer.
     * Also, since nsteps should be less than srcw then
     * (srcLoc + nsteps * srcInc) should not overflow either
     */
    nsteps = (((srcLimit - srcPos) << shift) - srcLoc + srcInc - 1) / srcInc;
    *pDstPos += nsteps;
    *pSrcLoc = (srcLoc + nsteps * srcInc) & ((1 << shift) - 1);
}

static void
Scale_adjustDn(jint srcPos, jint srcStart, jint srcLimit,
	       jint srcLoc, jint srcInc, jint shift,
	       jint dstPos, jint *pDstLimit)
{
    jint nsteps;

    if (srcLimit >= srcPos) return;
    /* The Overflow checking on this is nearly identical to adjustUp. */
    nsteps = (((srcLimit - srcStart) << shift) - srcLoc + srcInc - 1) / srcInc;
    *pDstLimit = dstPos + nsteps;
}

/*
 * Class:     sun_java2d_loops_ScaledBlit
 * Method:    Scale
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/SurfaceData;Ljava/awt/Composite;IIIIIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_loops_ScaledBlit_Scale
    (JNIEnv *env, jobject self,
     jobject srcData, jobject dstData,
     jobject comp, jobject clip,
     jint srcx, jint srcy, jint dstx, jint dsty,
     jint srcw, jint srch, jint dstw, jint dsth)
{
    SurfaceDataOps *srcOps;
    SurfaceDataOps *dstOps;
    SurfaceDataRasInfo srcInfo;
    SurfaceDataRasInfo dstInfo;
    NativePrimitive *pPrim;
    CompositeInfo compInfo;
    jint sxloc, syloc, sxinc, syinc, shift;
    double scale;
    RegionData clipInfo;
    jint dstFlags;

    pPrim = GetNativePrim(env, self);
    if (pPrim == NULL) {
	return;
    }
    if (pPrim->pCompType->getCompInfo != NULL) {
	(*pPrim->pCompType->getCompInfo)(env, &compInfo, comp);
    }
    if (Region_GetInfo(env, clip, &clipInfo)) {
	return;
    }

    srcOps = SurfaceData_GetOps(env, srcData);
    dstOps = SurfaceData_GetOps(env, dstData);
    if (srcOps == 0 || dstOps == 0) {
	return;
    }

    /*
     * Determine the precision to use for the fixed point math
     * for the coordinate scaling.
     * - OR together srcw and srch to get the MSB between the two
     * - Next shift it up until it goes negative
     * - Count the shifts and that will be the most accurate
     *   precision available for the fixed point math
     * - 1.0 will be (1 << shift)
     * - srcw & srch will be (srcw << shift) and (srch << shift)
     *   and will not overflow
     */
    sxloc = srcw | srch;
    shift = 0;
    while ((sxloc <<= 1) > 0) {
	shift++;
    }
    sxloc = (1 << shift);
    scale = ((double) (srch)) / ((double) (dsth));
    syinc = (int) (scale * sxloc);
    scale = ((double) (srcw)) / ((double) (dstw));
    sxinc = (int) (scale * sxloc);

    /*
     * Round by setting the initial sxyloc to half a destination
     * pixel which equals half of the x/y increments.
     */
    sxloc = sxinc / 2;
    syloc = syinc / 2;

    srcw += srcx;
    srch += srcy;
    srcInfo.bounds.x1 = srcx;
    srcInfo.bounds.y1 = srcy;
    srcInfo.bounds.x2 = srcw;
    srcInfo.bounds.y2 = srch;
    if (srcOps->Lock(env, srcOps, &srcInfo, pPrim->srcflags) != SD_SUCCESS) {
	return;
    }
    if (srcInfo.bounds.x2 <= srcInfo.bounds.x1 ||
	srcInfo.bounds.y2 <= srcInfo.bounds.y1)
    {
	SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
	return;
    }
    dstw += dstx;
    dsth += dsty;
    Scale_adjustUp(srcx, srcInfo.bounds.x1,
		   &sxloc, sxinc, shift,
		   &dstx);
    Scale_adjustUp(srcy, srcInfo.bounds.y1,
		   &syloc, syinc, shift,
		   &dsty);
    Scale_adjustDn(srcw, srcInfo.bounds.x1, srcInfo.bounds.x2,
		   sxloc, sxinc, shift,
		   dstx, &dstw);
    Scale_adjustDn(srch, srcInfo.bounds.y1, srcInfo.bounds.y2,
		   syloc, syinc, shift,
		   dsty, &dsth);

    dstInfo.bounds.x1 = dstx;
    dstInfo.bounds.y1 = dsty;
    dstInfo.bounds.x2 = dstw;
    dstInfo.bounds.y2 = dsth;
    SurfaceData_IntersectBounds(&dstInfo.bounds, &clipInfo.bounds);
    dstFlags = pPrim->dstflags;
    if (!Region_IsRectangular(&clipInfo)) {
	dstFlags |= SD_LOCK_PARTIAL_WRITE;
    }
    if (dstOps->Lock(env, dstOps, &dstInfo, dstFlags) != SD_SUCCESS) {
	SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
	return;
    }

    if (dstInfo.bounds.x2 > dstInfo.bounds.x1 &&
	dstInfo.bounds.y2 > dstInfo.bounds.y1)
    {
	srcOps->GetRasInfo(env, srcOps, &srcInfo);
	dstOps->GetRasInfo(env, dstOps, &dstInfo);
	if (srcInfo.rasBase && dstInfo.rasBase) {
	    SurfaceDataBounds span;
	    void *pSrc = PtrCoord(srcInfo.rasBase,
				  srcInfo.bounds.x1, srcInfo.pixelStride,
				  srcInfo.bounds.y1, srcInfo.scanStride);
	    Region_IntersectBounds(&clipInfo, &dstInfo.bounds);
	    Region_StartIteration(env, &clipInfo);
	    while (Region_NextIteration(&clipInfo, &span)) {
		jint width = span.x2 - span.x1;
		jint height = span.y2 - span.y1;
		jint tsxloc = sxloc;
		jint tsyloc = syloc;
		void *pDst = PtrCoord(dstInfo.rasBase,
				      span.x1, dstInfo.pixelStride,
				      span.y1, dstInfo.scanStride);
		if (span.y1 > dsty) {
		    tsyloc += syinc * (span.y1 - dsty);
		}
		if (span.x1 > dstx) {
		    tsxloc += sxinc * (span.x1 - dstx);
		}
		(*pPrim->funcs.scaledblit)(pSrc, pDst, width, height,
					   tsxloc, tsyloc, sxinc, syinc, shift,
					   &srcInfo, &dstInfo,
					   pPrim, &compInfo);
	    }
	    Region_EndIteration(env, &clipInfo);
	}
	SurfaceData_InvokeRelease(env, dstOps, &dstInfo);
	SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
    }
    SurfaceData_InvokeUnlock(env, dstOps, &dstInfo);
    SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
}
