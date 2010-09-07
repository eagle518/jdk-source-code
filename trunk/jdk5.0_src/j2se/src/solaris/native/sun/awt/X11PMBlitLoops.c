/*
 * @(#)X11PMBlitLoops.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <jni.h>
#include "X11SurfaceData.h"
#include "Region.h"

JNIEXPORT void JNICALL
Java_sun_awt_X11PMBlitLoops_Blit
    (JNIEnv *env, jobject joSelf,
     jobject srcData, jobject dstData,
     jobject composite, jobject clip,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height)
{
#ifndef HEADLESS
    X11SDOps *srcXsdo, *dstXsdo;
    SurfaceDataBounds span;
    RegionData clipInfo;
    GC xgc;

    if (width <= 0 || height <= 0) {
	return;
    }

    srcXsdo = X11SurfaceData_GetOps(env, srcData);
    if (srcXsdo == NULL) {
	return;
    }
    dstXsdo = X11SurfaceData_GetOps(env, dstData);
    if (dstXsdo == NULL) {
	return;
    }
    if (Region_GetInfo(env, clip, &clipInfo)) {
	return;
    }

    xgc = dstXsdo->GetGC(env, dstXsdo, NULL, NULL, dstXsdo->lastpixel);
    if (xgc == NULL) {
	return;
    }

#ifdef MITSHM
    if (srcXsdo->isPixmap) {
	X11SD_UnPuntPixmap(srcXsdo);
    }
#endif /* MITSHM */

    if (srcXsdo->bitmask != 0) {
	XSetClipOrigin(awt_display, xgc, dstx - srcx, dsty - srcy);
	XSetClipMask(awt_display, xgc, srcXsdo->bitmask);
    }
    span.x1 = dstx;
    span.y1 = dsty;
    span.x2 = dstx + width;
    span.y2 = dsty + height;
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
    dstXsdo->ReleaseGC(env, dstXsdo, xgc);
#endif /* !HEADLESS */
}

JNIEXPORT void JNICALL
Java_sun_awt_X11PMBlitBgLoops_nativeBlitBg
    (JNIEnv *env, jobject joSelf,
     jobject srcData, jobject dstData,
     jobject composite, jobject clip, jint pixel,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height)
{
#ifndef HEADLESS
    X11SDOps *srcXsdo, *dstXsdo;
    GC dstGC;
    Drawable srcDrawable;

    if (width <= 0 || height <= 0) {
	return;
    }

    srcXsdo = X11SurfaceData_GetOps(env, srcData);
    if (srcXsdo == NULL) {
	return;
    }
    dstXsdo = X11SurfaceData_GetOps(env, dstData);
    if (dstXsdo == NULL) {
	return;
    }

    dstGC = dstXsdo->GetGC(env, dstXsdo, clip, NULL, pixel);
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
	dstXsdo->ReleaseGC(env, dstXsdo, dstGC);
	return;
    }

    /* do an unmasked copy as we've already filled transparent 
       pixels of the source image with the desired color */
    XCopyArea(awt_display, srcDrawable, dstXsdo->drawable, dstGC,
	      srcx, srcy, width, height, dstx, dsty);

    srcXsdo->ReleasePixmapWithBg(env, srcXsdo);
    dstXsdo->ReleaseGC(env, dstXsdo, dstGC);
#endif /* !HEADLESS */
}
