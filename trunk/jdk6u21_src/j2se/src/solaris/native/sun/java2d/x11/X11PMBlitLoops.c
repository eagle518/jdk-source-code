/*
 * @(#)X11PMBlitLoops.c	1.17 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <jni.h>
#include <jlong.h>
#include "X11SurfaceData.h"
#include "Region.h"

JNIEXPORT void JNICALL
Java_sun_java2d_x11_X11PMBlitLoops_nativeBlit
    (JNIEnv *env, jobject joSelf,
     jlong srcData, jlong dstData,
     jlong gc, jobject clip,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height)
{
#ifndef HEADLESS
    X11SDOps *srcXsdo, *dstXsdo;
    SurfaceDataBounds span, srcBounds;
    RegionData clipInfo;
    GC xgc;

    if (width <= 0 || height <= 0) {
	return;
    }

    srcXsdo = (X11SDOps *)jlong_to_ptr(srcData);
    if (srcXsdo == NULL) {
	return;
    }
    dstXsdo = (X11SDOps *)jlong_to_ptr(dstData);
    if (dstXsdo == NULL) {
	return;
    }
    if (Region_GetInfo(env, clip, &clipInfo)) {
	return;
    }

    xgc = (GC)gc;
    if (xgc == NULL) {
	return;
    }

#ifdef MITSHM
    if (srcXsdo->isPixmap) {
	X11SD_UnPuntPixmap(srcXsdo);
    }
#endif /* MITSHM */

    /* clip the source rect to the source pixmap's dimensions */
    srcBounds.x1 = srcx;
    srcBounds.y1 = srcy;
    srcBounds.x2 = srcx + width;
    srcBounds.y2 = srcy + height;
    SurfaceData_IntersectBoundsXYXY(&srcBounds, 
                                    0, 0, srcXsdo->pmWidth, srcXsdo->pmHeight);
    span.x1 = dstx;
    span.y1 = dsty;
    span.x2 = dstx + width;
    span.y2 = dsty + height;

    /* intersect the source and dest rects */
    SurfaceData_IntersectBlitBounds(&srcBounds, &span, 
                                    dstx - srcx, dsty - srcy);
    srcx = srcBounds.x1;
    srcy = srcBounds.y1;
    dstx = span.x1;
    dsty = span.y1;

    if (srcXsdo->bitmask != 0) {
	XSetClipOrigin(awt_display, xgc, dstx - srcx, dsty - srcy);
	XSetClipMask(awt_display, xgc, srcXsdo->bitmask);
    }

    Region_IntersectBounds(&clipInfo, &span);
    if (!Region_IsEmpty(&clipInfo)) {
	Region_StartIteration(env, &clipInfo);
	srcx -= dstx;
	srcy -= dsty;
	while (Region_NextIteration(&clipInfo, &span)) {
	    XCopyArea(awt_display, srcXsdo->drawable, dstXsdo->drawable, xgc,
		      srcx + span.x1, srcy + span.y1,
		      span.x2 - span.x1, span.y2 - span.y1,
		      span.x1, span.y1);
	}
	Region_EndIteration(env, &clipInfo);
    }

    if (srcXsdo->bitmask != 0) {
	XSetClipMask(awt_display, xgc, None);
    }

#ifdef MITSHM
    if (srcXsdo->shmPMData.usingShmPixmap) { 
	srcXsdo->shmPMData.xRequestSent = JNI_TRUE;
    }
#endif /* MITSHM */
    X11SD_DirectRenderNotify(env, dstXsdo);
#endif /* !HEADLESS */
}

JNIEXPORT void JNICALL
Java_sun_java2d_x11_X11PMBlitBgLoops_nativeBlitBg
    (JNIEnv *env, jobject joSelf,
     jlong srcData, jlong dstData,
     jlong xgc, jint pixel,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height)
{
#ifndef HEADLESS
    X11SDOps *srcXsdo, *dstXsdo;
    GC dstGC;
    SurfaceDataBounds dstBounds, srcBounds;
    Drawable srcDrawable;

    if (width <= 0 || height <= 0) {
	return;
    }

    srcXsdo = (X11SDOps *)jlong_to_ptr(srcData);
    if (srcXsdo == NULL) {
	return;
    }
    dstXsdo = (X11SDOps *)jlong_to_ptr(dstData);
    if (dstXsdo == NULL) {
	return;
    }

    dstGC = (GC)xgc;
    if (dstGC == NULL) {
	return;
    }

#ifdef MITSHM
    if (srcXsdo->isPixmap) {
	X11SD_UnPuntPixmap(srcXsdo);
    }
#endif /* MITSHM */
    
    srcDrawable = srcXsdo->GetPixmapWithBg(env, srcXsdo, pixel);
    if (srcDrawable == 0) {
	return;
    }

    /* clip the source rect to the source pixmap's dimensions */
    srcBounds.x1 = srcx;
    srcBounds.y1 = srcy;
    srcBounds.x2 = srcx + width;
    srcBounds.y2 = srcy + height;
    SurfaceData_IntersectBoundsXYXY(&srcBounds, 
                                    0, 0, srcXsdo->pmWidth, srcXsdo->pmHeight);
    dstBounds.x1 = dstx;
    dstBounds.y1 = dsty;
    dstBounds.x2 = dstx + width;
    dstBounds.y2 = dsty + height;

    /* intersect the source and dest rects */
    SurfaceData_IntersectBlitBounds(&srcBounds, &dstBounds, 
                                    dstx - srcx, dsty - srcy);
    srcx = srcBounds.x1;
    srcy = srcBounds.y1;
    dstx = dstBounds.x1;
    dsty = dstBounds.y1;
    width = srcBounds.x2 - srcBounds.x1;
    height = srcBounds.y2 - srcBounds.y1;

    /* do an unmasked copy as we've already filled transparent 
       pixels of the source image with the desired color */
    XCopyArea(awt_display, srcDrawable, dstXsdo->drawable, dstGC,
	      srcx, srcy, width, height, dstx, dsty);

    srcXsdo->ReleasePixmapWithBg(env, srcXsdo);
    X11SD_DirectRenderNotify(env, dstXsdo);
#endif /* !HEADLESS */
}
