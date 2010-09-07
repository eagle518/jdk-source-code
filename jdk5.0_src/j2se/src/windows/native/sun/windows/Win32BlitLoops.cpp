/*
 * @(#)Win32BlitLoops.cpp	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <jni.h>
#include <sun_awt_windows_Win32BlitLoops.h> 
#include <sun_awt_windows_Win32ScaleLoops.h> 
#include "ddrawUtils.h"
#include "GraphicsPrimitiveMgr.h"
#include "Region.h"


extern int currNumDevices;
extern CriticalSection windowMoveLock;

extern "C" {

/**
 * Return TRUE if rCheck is contained within rContainer
 */
INLINE BOOL RectInRect(RECT *rCheck, RECT *rContainer)
{
    // Assumption: left <= right, top <= bottom
    if (rCheck->left >= rContainer->left &&
	rCheck->right <= rContainer->right &&
	rCheck->top >= rContainer->top &&
	rCheck->bottom <= rContainer->bottom)
    {
	return TRUE;
    } else {
	return FALSE;
    }
}

/**
 * Returns whether the given rectangle (in screen-relative
 * coords) is within the rectangle of the given device.
 * NOTE: A side-effect of this function is offsetting the
 * rectangle by the left/top of the monitor rectangle.
 */
INLINE BOOL RectInDevice(RECT *rect, AwtWin32GraphicsDevice *device) 
{
    MONITOR_INFO *mi = device->GetMonitorInfo();
    ::OffsetRect(rect, mi->rMonitor.left, mi->rMonitor.top);
    if (!RectInRect(rect, &mi->rMonitor)) {
	return TRUE;
    }
    return FALSE;
}

/**
 * Need to handle Blt to other devices iff:
 *    - there are >1 devices on the system
 *    - at least one of src/dest is an onscreen window
 *    - the onscreen window overlaps with
 *    a monitor which is not the monitor associated with the window
 */
void MultimonBlt(JNIEnv *env, Win32SDOps *wsdoSrc, Win32SDOps *wsdoDst,
		 jobject clip,
		 jint srcx, jint srcy,
		 jint dstx, jint dsty,
		 RECT *rSrc, RECT *rDst)
{
    DTRACE_PRINTLN4("MM Blt: srcx, srcy, dy, dy, w, h = %d, %d, %d, %d",
	srcx, srcy, dsty, dsty);
    int currentDevice = -1;
    RECT rectToIntersect;

    if (!(wsdoSrc->window || wsdoDst->window))
    {
	// Neither surface is onscreen: nothing to do
	return;
    }
    BOOL doGdiBlt = FALSE;
    if (wsdoSrc->window) {
	doGdiBlt = RectInDevice(rSrc, wsdoSrc->device);
	if (doGdiBlt) {
	    currentDevice = wsdoSrc->device->GetDeviceIndex();
	    rectToIntersect = *rSrc;
	}
    } else if (wsdoDst->window) {
	doGdiBlt = RectInDevice(rDst, wsdoDst->device);
	if (doGdiBlt) {
	    currentDevice = wsdoDst->device->GetDeviceIndex();
	    rectToIntersect = *rDst;
	}
    }
    if (doGdiBlt) {
	// Need to invoke Setup functions to setup HDCs because we used
	// the NoSetup versions of GetOps for performance reasons
	SurfaceData_InvokeSetup(env, (SurfaceDataOps*)wsdoSrc);
	SurfaceData_InvokeSetup(env, (SurfaceDataOps*)wsdoDst);
	HDC hdcSrc = wsdoSrc->GetDC(env, wsdoSrc, 0, NULL, NULL, NULL, 0);
	if (!hdcSrc) {
	    J2dTraceLn(J2D_TRACE_ERROR, "Null src HDC in MultimonBlt");
	    return;
	}
	HDC hdcDst = wsdoDst->GetDC(env, wsdoDst, 0, NULL, clip, NULL, 0);
	if (!hdcDst) {
	    J2dTraceLn(J2D_TRACE_ERROR, "Null dst HDC in MultimonBlt");
	    wsdoSrc->ReleaseDC(env, wsdoSrc, hdcSrc);
	    return;
	}
	for (int i = 0; i < currNumDevices; ++i) {
	    // Assumption: can't end up here for copies between two
	    // different windows; it must be a copy between offscreen
	    // surfaces or a window and an offscreen surface.  We've
	    // already handled the Blt to window on the window's native
	    // GraphicsDevice, so skip that device now.
	    if (i == currentDevice) {
		continue;
	    }
	    MONITOR_INFO *mi = AwtWin32GraphicsDevice::GetMonitorInfo(i);
	    RECT rIntersect;
	    ::IntersectRect(&rIntersect, &rectToIntersect, &mi->rMonitor);
	    if (!::IsRectEmpty(&rIntersect)) {
		int newSrcX = srcx + (rIntersect.left - rectToIntersect.left);
		int newSrcY = srcy + (rIntersect.top - rectToIntersect.top);
		int newDstX = dstx + (rIntersect.left - rectToIntersect.left);
		int newDstY = dsty + (rIntersect.top - rectToIntersect.top);
		int newW = rIntersect.right - rIntersect.left;
		int newH = rIntersect.bottom - rIntersect.top;
		::BitBlt(hdcDst, newDstX, newDstY, newW, newH, hdcSrc, 
		    newSrcX, newSrcY, SRCCOPY);
	    }
	}
	wsdoSrc->ReleaseDC(env, wsdoSrc, hdcSrc);
	wsdoDst->ReleaseDC(env, wsdoDst, hdcDst);
    }
}

#define CLIP2RECTS_1PARAM(r1, r2, param, comp, lim) \
    do { \
	if (r1.param comp lim) { \
	    r2.param += lim - r1.param; \
	    r1.param = lim; \
	} \
    } while (0)

#define CLIP2RECTS(r1, L, T, R, B, r2) \
    do { \
	CLIP2RECTS_1PARAM(r1, r2, left, <, L); \
	CLIP2RECTS_1PARAM(r1, r2, top, <, T); \
	CLIP2RECTS_1PARAM(r1, r2, right, >, R); \
	CLIP2RECTS_1PARAM(r1, r2, bottom, >, B); \
    } while(0)

JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32BlitLoops_Blit
    (JNIEnv *env, jobject joSelf,
     jobject srcData, jobject dstData,
     jobject composite, jobject clip,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height)
{
    DTRACE_PRINTLN("Win32BlitLoops::BlitNative");
    POINT ptDst = {0, 0};
    POINT ptSrc = {0, 0};
    Win32SDOps *wsdoSrc = Win32SurfaceData_GetOpsNoSetup(env, srcData);
    Win32SDOps *wsdoDst = Win32SurfaceData_GetOpsNoSetup(env, dstData);
    RegionData clipInfo;

    if (!wsdoSrc->ddInstance || !wsdoDst->ddInstance) {
	// Some situations can cause us to fail on primary
	// creation, resulting in null lpSurface and null ddInstance
	// for a Win32Surface object.. Just noop this call in that case.
	return;
    }

    RECT rSrc = {srcx, srcy, srcx + width, srcy + height};
    RECT rDst = {dstx, dsty, dstx + width, dsty + height};
    if (Region_GetInfo(env, clip, &clipInfo)) {
	return;
    }

    /* If dst and/or src are offscreen surfaces, need to make sure
       that Blt is within the boundaries of those surfaces.  If not,
       clip the surface in question and also clip the other 
       surface by the same amount.
     */
    if (!wsdoDst->window) {
	CLIP2RECTS(rDst, 0, 0, wsdoDst->w, wsdoDst->h, rSrc);
    }
    CLIP2RECTS(rDst,
	       clipInfo.bounds.x1, clipInfo.bounds.y1,
	       clipInfo.bounds.x2, clipInfo.bounds.y2,
	       rSrc);
    if (!wsdoSrc->window) {
	CLIP2RECTS(rSrc, 0, 0, wsdoSrc->w, wsdoSrc->h, rDst);
    }
    Region_IntersectBoundsXYXY(&clipInfo,
			       rDst.left, rDst.top,
			       rDst.right, rDst.bottom);
    if (Region_IsEmpty(&clipInfo)) {
	return;
    }
    if (wsdoDst->window || wsdoSrc->window) {
	if ((wsdoDst->window && !::IsWindowVisible(wsdoDst->window)) ||
	    (wsdoSrc->window && !::IsWindowVisible(wsdoSrc->window)))
	{
	    return;
	}
	// The windowMoveLock CriticalSection ensures that a window cannot
	// move while we are in the middle of copying pixels into it.  See
	// the WM_WINDOWPOSCHANGING code in awt_Component.cpp for more
	// information.
	windowMoveLock.Enter();
	if (wsdoDst->window) {
	    ::ClientToScreen(wsdoDst->window, &ptDst);
	    MONITOR_INFO *mi = wsdoDst->device->GetMonitorInfo();
	    ptDst.x -= wsdoDst->insets.left;
	    ptDst.y -= wsdoDst->insets.top;
	    ptDst.x -= mi->rMonitor.left;
	    ptDst.y -= mi->rMonitor.top;
	    ::OffsetRect(&rDst, ptDst.x, ptDst.y);
	}
	if (wsdoSrc->window) {
	    MONITOR_INFO *mi = wsdoDst->device->GetMonitorInfo();
	    ::ClientToScreen(wsdoSrc->window, &ptSrc);
	    ptSrc.x -= wsdoSrc->insets.left;
	    ptSrc.y -= wsdoSrc->insets.top;
	    ptSrc.x -= mi->rMonitor.left;
	    ptSrc.y -= mi->rMonitor.top;
	    ::OffsetRect(&rSrc, ptSrc.x, ptSrc.y);
	}
    }
    if (Region_IsRectangular(&clipInfo)) {
	DDBlt(env, wsdoSrc, wsdoDst, &rDst, &rSrc); 
    } else {
	SurfaceDataBounds span;
	RECT rSrcSpan, rDstSpan;
	ptSrc.x += srcx - dstx;
	ptSrc.y += srcy - dsty;
	Region_StartIteration(env, &clipInfo);
	while (Region_NextIteration(&clipInfo, &span)) {
	    ::SetRect(&rDstSpan, span.x1, span.y1, span.x2, span.y2);
	    ::CopyRect(&rSrcSpan, &rDstSpan);
	    ::OffsetRect(&rDstSpan, ptDst.x, ptDst.y);
	    ::OffsetRect(&rSrcSpan, ptSrc.x, ptSrc.y);
	    DDBlt(env, wsdoSrc, wsdoDst, &rDstSpan, &rSrcSpan); 
	}
	Region_EndIteration(env, &clipInfo);
    }
    if (wsdoDst->window || wsdoSrc->window) {
	windowMoveLock.Leave();
    }

    if (currNumDevices > 1) {
	// Also need to handle Blit in multimon case, where part of the
	// source or dest lies on a different device
	MultimonBlt(env, wsdoSrc, wsdoDst, clip, srcx, srcy, dstx, dsty, 
		    &rSrc, &rDst);
    }
}


JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32ScaleLoops_Scale
    (JNIEnv *env, jobject joSelf,
     jobject srcData, jobject dstData,
     jobject composite,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint srcWidth, jint srcHeight,
     jint dstWidth, jint dstHeight)
{
    DTRACE_PRINTLN("Win32BlitLoops::Scale");
    POINT ptDst = {0, 0};
    POINT ptSrc = {0, 0};
    Win32SDOps *wsdoSrc = Win32SurfaceData_GetOpsNoSetup(env, srcData);
    Win32SDOps *wsdoDst = Win32SurfaceData_GetOpsNoSetup(env, dstData);

    if (!wsdoSrc->ddInstance || !wsdoDst->ddInstance) {
	// Some situations can cause us to fail on primary
	// creation, resulting in null lpSurface and null ddInstance
	// for a Win32Surface object.. Just noop this call in that case.
	return;
    }

    RECT rSrc = {srcx, srcy, srcx + srcWidth, srcy + srcHeight};
    RECT rDst = {dstx, dsty, dstx + dstWidth, dsty + dstHeight};

    /* If dst and/or src are offscreen surfaces, need to make sure
       that Blt is within the boundaries of those surfaces.  If not,
       clip the surface in question and also rescale the other 
       surface according to the new scaling rectangle.
     */
    if (!wsdoDst->window &&
	(dstx < 0 || dsty < 0 || 
	 rDst.right > wsdoDst->w || rDst.bottom > wsdoDst->h))
    {
	RECT newRDst;
	newRDst.left = max(0, rDst.left); 
	newRDst.top = max(0, rDst.top);
	newRDst.right = min(wsdoDst->w, rDst.right);
	newRDst.bottom = min(wsdoDst->h, rDst.bottom);
	double srcDstScaleW = (double)srcWidth/(double)dstWidth;
	double srcDstScaleH = (double)srcHeight/(double)dstHeight;
	rSrc.left += (int)(srcDstScaleW * (newRDst.left - rDst.left));
	rSrc.top += (int)(srcDstScaleH * (newRDst.top - rDst.top));
	rSrc.right += (int)(srcDstScaleW * (newRDst.right - rDst.right));
	rSrc.bottom += (int)(srcDstScaleH * (newRDst.bottom - rDst.bottom));
	rDst = newRDst;
    }
    if (!wsdoSrc->window &&
	(srcx < 0 || srcy < 0 || 
	 rSrc.right > wsdoSrc->w || rSrc.bottom > wsdoSrc->h))
    {
	RECT newRSrc;
	newRSrc.left = max(0, rSrc.left); 
	newRSrc.top = max(0, rSrc.top);
	newRSrc.right = min(wsdoSrc->w, rSrc.right);
	newRSrc.bottom = min(wsdoSrc->h, rSrc.bottom);
	double dstSrcScaleW = (double)dstWidth/(double)srcWidth;
	double dstSrcScaleH = (double)dstHeight/(double)srcHeight;
	rDst.left += (int)(dstSrcScaleW * (newRSrc.left - rSrc.left));
	rDst.top += (int)(dstSrcScaleH * (newRSrc.top - rSrc.top));
	rDst.right += (int)(dstSrcScaleW * (newRSrc.right - rSrc.right));
	rDst.bottom += (int)(dstSrcScaleH * (newRSrc.bottom - rSrc.bottom));
	rSrc = newRSrc;
    }
    if (wsdoDst->window || wsdoSrc->window) {
	if ((wsdoDst->window && !::IsWindowVisible(wsdoDst->window)) ||
	    (wsdoSrc->window && !::IsWindowVisible(wsdoSrc->window)))
	{
	    return;
	}
	// The windowMoveLock CriticalSection ensures that a window cannot
	// move while we are in the middle of copying pixels into it.  See
	// the WM_WINDOWPOSCHANGING code in awt_Component.cpp for more
	// information.
	windowMoveLock.Enter();
	if (wsdoDst->window) {
	    ::ClientToScreen(wsdoDst->window, &ptDst);
	    MONITOR_INFO *mi = wsdoDst->device->GetMonitorInfo();
	    ptDst.x -= wsdoDst->insets.left;
	    ptDst.y -= wsdoDst->insets.top;
	    ptDst.x -= mi->rMonitor.left;
	    ptDst.y -= mi->rMonitor.top;
	    ::OffsetRect(&rDst, ptDst.x, ptDst.y);
	}
	if (wsdoSrc->window) {
	    MONITOR_INFO *mi = wsdoDst->device->GetMonitorInfo();
	    ::ClientToScreen(wsdoSrc->window, &ptSrc);
	    ptSrc.x -= wsdoSrc->insets.left;
	    ptSrc.y -= wsdoSrc->insets.top;
	    ptSrc.x -= mi->rMonitor.left;
	    ptSrc.y -= mi->rMonitor.top;
	    ::OffsetRect(&rSrc, ptSrc.x, ptSrc.y);
	}
    }
    DDBlt(env, wsdoSrc, wsdoDst, &rDst, &rSrc); 
    if (wsdoDst->window || wsdoSrc->window) {
	windowMoveLock.Leave();
    }

    if (currNumDevices > 1) {
	// Also need to handle Blit in multimon case, where part of the
	// source or dest lies on a different device
	MultimonBlt(env, wsdoSrc, wsdoDst, NULL, srcx, srcy, dstx, dsty, 
		    &rSrc, &rDst);
    }
}

/*
 * Class:     sun_awt_windows_D3DBlitLoops_Blit
 * Method:    nativeBlit
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/SurfaceData;IIIIIIZ)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_D3DBlitLoops_Blit
    (JNIEnv *env, jobject joSelf,
     jobject srcData, jobject dstData,
     jobject composite, jobject clip, 
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height)
{
    DTRACE_PRINTLN("D3DBlitLoops::Blit");

    POINT ptDst = {0, 0};
    POINT ptSrc = {0, 0};
    Win32SDOps *wsdoSrc = Win32SurfaceData_GetOpsNoSetup(env, srcData);
    Win32SDOps *wsdoDst = Win32SurfaceData_GetOpsNoSetup(env, dstData);
    RegionData clipInfo;
    CompositeInfo compInfo;

    if (!wsdoSrc->ddInstance || !wsdoDst->ddInstance) {
	// Some situations can cause us to fail on primary
	// creation, resulting in null lpSurface and null ddInstance
	// for a Win32Surface object.. Just noop this call in that case.
	return;
    }

    GrPrim_CompGetAlphaInfo(env, &compInfo, composite);

    RECT rSrc = {srcx, srcy, srcx + width, srcy + height};
    RECT rDst = {dstx, dsty, dstx + width, dsty + height};
    if (Region_GetInfo(env, clip, &clipInfo)) {
	return;
    }

    // If dst and/or src are offscreen surfaces, need to make sure
    // that Blt is within the boundaries of those surfaces.  If not,
    // clip the surface in question and also rescale the other 
    // surface according to the new scaling rectangle.
    if (!wsdoDst->window) {
	CLIP2RECTS(rDst, 0, 0, wsdoDst->w, wsdoDst->h, rSrc);
    }
    CLIP2RECTS(rDst,
	       clipInfo.bounds.x1, clipInfo.bounds.y1,
	       clipInfo.bounds.x2, clipInfo.bounds.y2,
	       rSrc);
    // the source is always an offscreen surface
    CLIP2RECTS(rSrc, 0, 0, wsdoSrc->w, wsdoSrc->h, rDst);
    Region_IntersectBoundsXYXY(&clipInfo,
			       rDst.left, rDst.top,
			       rDst.right, rDst.bottom);
    if (Region_IsEmpty(&clipInfo)) {
	return;
    }

    if (wsdoDst->window) {
	if (wsdoDst->window && !::IsWindowVisible(wsdoDst->window))
	{
	    return;
	}
	// The windowMoveLock CriticalSection ensures that a window cannot
	// move while we are in the middle of copying pixels into it.  See
	// the WM_WINDOWPOSCHANGING code in awt_Component.cpp for more
	// information.
	windowMoveLock.Enter();
	if (wsdoDst->window) {
	    ::ClientToScreen(wsdoDst->window, &ptDst);
	    MONITOR_INFO *mi = wsdoDst->device->GetMonitorInfo();
	    ptDst.x -= wsdoDst->insets.left;
	    ptDst.y -= wsdoDst->insets.top;
	    ptDst.x -= mi->rMonitor.left;
	    ptDst.y -= mi->rMonitor.top;
	    ::OffsetRect(&rDst, ptDst.x, ptDst.y);
	}
    }
    if (Region_IsRectangular(&clipInfo)) {
	DDBlt(env, wsdoSrc, wsdoDst, &rDst, &rSrc, &compInfo); 
    } else {
	SurfaceDataBounds span;
	RECT rSrcSpan, rDstSpan;
	ptSrc.x += srcx - dstx;
	ptSrc.y += srcy - dsty;
	Region_StartIteration(env, &clipInfo);
	while (Region_NextIteration(&clipInfo, &span)) {
	    ::SetRect(&rDstSpan, span.x1, span.y1, span.x2, span.y2);
	    ::CopyRect(&rSrcSpan, &rDstSpan);
	    ::OffsetRect(&rDstSpan, ptDst.x, ptDst.y);
	    ::OffsetRect(&rSrcSpan, ptSrc.x, ptSrc.y);
	    DDBlt(env, wsdoSrc, wsdoDst, &rDstSpan, &rSrcSpan, &compInfo); 
	}
	Region_EndIteration(env, &clipInfo);
    }

    if (wsdoDst->window) {
	windowMoveLock.Leave();
    }

    // REMIND: handle multimon here somehow
    return;
}

}
