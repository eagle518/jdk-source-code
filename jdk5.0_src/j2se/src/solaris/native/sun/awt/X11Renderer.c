/*
 * @(#)X11Renderer.c	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_X11Renderer.h"

#include "X11SurfaceData.h"
#include "SpanIterator.h"
#include "Region.h"
#include "Trace.h"
#include "Disposer.h"

#include <jlong.h>

GeneralDisposeFunc X11Renderer_XFreeGC;

#ifndef HEADLESS
#define POLYTEMPSIZE	(int)(256 / sizeof(XPoint))
#define ABS(n)		(((n) < 0) ? -(n) : (n))
#define CLAMP_TO_SHORT(x)   (((x) > 32767) ? 32767 : ((x) < -32768) ? -32768 : (x))
#define CLAMP_TO_USHORT(x)  (((x) > 65535) ? 65535 : ((x) < 0) ? 0 : (x))

static void
awt_drawArc(JNIEnv * env, jint drawable, GC xgc,
            int x, int y, int w, int h,
            int startAngle, int endAngle,
            int filled)
{
    int s, e;

    if (w < 0 || h < 0) {
        return;
    }
    if (endAngle >= 360 || endAngle <= -360) {
        s = 0;
        e = 360 * 64;
    } else {
        s = (startAngle % 360) * 64;
        e = endAngle * 64;
    }
    if (filled == 0) {
        XDrawArc(awt_display, drawable, xgc, x, y, w, h, s, e);
    } else {
        XFillArc(awt_display, drawable, xgc, x, y, w, h, s, e);
    }
}

/*
 * Copy vertices from xcoordsArray and ycoordsArray to a buffer
 * of XPoint structures, translating by transx and transy and
 * collapsing empty segments out of the list as we go.
 * The number of points to be converted should be guaranteed
 * to be more than 2 by the caller and is stored at *pNpoints.
 * The resulting number of uncollapsed unique translated vertices
 * will be stored back into the location *pNpoints.
 * The points pointer is guaranteed to be pointing to an area of
 * memory large enough for POLYTEMPSIZE points and a larger
 * area of memory is allocated (and returned) if that is not enough.
 */
static XPoint *
transformPoints(JNIEnv * env,
                jintArray xcoordsArray, jintArray ycoordsArray,
		jint transx, jint transy,
                XPoint * points, int *pNpoints, int close)
{
    int npoints = *pNpoints;
    jint *xcoords, *ycoords;

    xcoords = (jint *)
	(*env)->GetPrimitiveArrayCritical(env, xcoordsArray, NULL);
    if (xcoords == NULL) {
        return 0;
    }
    
    ycoords = (jint *)
	(*env)->GetPrimitiveArrayCritical(env, ycoordsArray, NULL);
    if (ycoords == NULL) {
        (*env)->ReleasePrimitiveArrayCritical(env, xcoordsArray, xcoords,
                                              JNI_ABORT);
        return 0;
    }

    if (close) {
        close = (xcoords[npoints - 1] != xcoords[0] ||
		 ycoords[npoints - 1] != ycoords[0]);
        if (close) {
            npoints++;
        }
    }
    if (npoints > POLYTEMPSIZE) {
        points = (XPoint *) malloc(sizeof(XPoint) * npoints);
    }
    if (points != NULL) {
	int in, out;
	int oldx = CLAMP_TO_SHORT(xcoords[0] + transx);
	int oldy = CLAMP_TO_SHORT(ycoords[0] + transy);
	points[0].x = oldx;
	points[0].y = oldy;
	if (close) {
	    npoints--;
	}
	for (in = 1, out = 1; in < npoints; in++) {
	    int newx = CLAMP_TO_SHORT(xcoords[in] + transx);
	    int newy = CLAMP_TO_SHORT(ycoords[in] + transy);
	    if (newx != oldx || newy != oldy) {
		points[out].x = newx;
		points[out].y = newy;
		out++;
		oldx = newx;
		oldy = newy;
	    }
	}
	if (out == 1) {
	    points[1].x = oldx;
	    points[1].y = oldy;
	    out = 2;
	} else if (close) {
	    points[out++] = points[0];
	}
	*pNpoints = out;
    }

    (*env)->ReleasePrimitiveArrayCritical(env, xcoordsArray, xcoords,
                                          JNI_ABORT);
    (*env)->ReleasePrimitiveArrayCritical(env, ycoordsArray, ycoords,
                                          JNI_ABORT);

    return points;
}
#endif /* !HEADLESS */

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XCreateGC
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_sun_awt_X11Renderer_XCreateGC
    (JNIEnv *env, jobject xr, jlong pXSData)
{
    jlong ret;
    X11SDOps *xsdo;
#ifndef HEADLESS

    J2dTraceLn(J2D_TRACE_INFO, "in X11Renderer_XCreateGC");

    xsdo = (X11SDOps *) pXSData;
    if (xsdo == NULL) {
	return 0L;
    }

    ret = (jlong) XCreateGC(awt_display, xsdo->drawable, 0, NULL);
    Disposer_AddRecord(env, xr, X11Renderer_XFreeGC, ret);
#else /* !HEADLESS */
    ret = 0L;
#endif /* !HEADLESS */

    return ret;
}

void X11Renderer_XFreeGC(JNIEnv *env, jlong xgc)
{
#ifndef HEADLESS
    J2dTraceLn(J2D_TRACE_INFO, "in X11Renderer_XFreeGC");
    AWT_LOCK();
    XFreeGC(awt_display, (GC) xgc);
    AWT_UNLOCK();
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XSetClip
 * Signature: (JIIIILsun/java2d/pipe/Region;)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XSetClip
    (JNIEnv *env, jclass xr, jlong xgc,
     jint x1, jint y1, jint x2, jint y2,
     jobject complexclip)
{
#ifndef HEADLESS
    RegionData clipInfo;
    SurfaceDataBounds span;
    XRectangle rects[256];
    XRectangle *pRect = rects;
    int i, numrects;

    J2dTraceLn(J2D_TRACE_INFO, "in X11Renderer_XSetClip");
    if (complexclip == NULL) {
	rects[0].x = x1;
	rects[0].y = y1;
	rects[0].width = x2 - x1;
	rects[0].height = y2 - y1;
	numrects = 1;
    } else {
	if (Region_GetInfo(env, complexclip, &clipInfo)) {
	    /* return; REMIND: What to do here? */
	}
	Region_StartIteration(env, &clipInfo);
	numrects = Region_CountIterationRects(&clipInfo);
	if (numrects > 256) {
	    pRect = (XRectangle *) malloc(numrects * sizeof(XRectangle));
	}
	i = 0;
	while (Region_NextIteration(&clipInfo, &span)) {
	    pRect[i].x = span.x1;
	    pRect[i].y = span.y1;
	    pRect[i].width = span.x2 - span.x1;
	    pRect[i].height = span.y2 - span.y1;
	    i++;
	}
	Region_EndIteration(env, &clipInfo);
    }
    XSetClipRectangles(awt_display, (GC) xgc, 0, 0, pRect, numrects, YXBanded);
    if (pRect != rects) {
	free(pRect);
    }
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XSetCopyMode
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XSetCopyMode
    (JNIEnv *env, jclass xr, jlong xgc)
{
#ifndef HEADLESS
    J2dTraceLn(J2D_TRACE_INFO, "in X11Renderer_XSetCopyMode");
    XSetFunction(awt_display, (GC) xgc, GXcopy);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XSetXorMode
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XSetXorMode
    (JNIEnv *env, jclass xr, jlong xgc)
{
#ifndef HEADLESS
    J2dTraceLn(J2D_TRACE_INFO, "in X11Renderer_XSetXorMode");
    XSetFunction(awt_display, (GC) xgc, GXxor);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XSetForeground
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XSetForeground
    (JNIEnv *env, jclass xr, jlong xgc, jint pixel)
{
#ifndef HEADLESS
    J2dTraceLn(J2D_TRACE_INFO, "in X11Renderer_XSetForeground");
    XSetForeground(awt_display, (GC) xgc, pixel);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XDrawLine
 * Signature: (IJIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XDrawLine
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint x1, jint y1, jint x2, jint y2)
{
#ifndef HEADLESS
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL) {
	return;
    }

    XDrawLine(awt_display, xsdo->drawable, (GC) xgc, 
	      CLAMP_TO_SHORT(x1), CLAMP_TO_SHORT(y1), 
	      CLAMP_TO_SHORT(x2), CLAMP_TO_SHORT(y2));
    X11SD_DirectRenderNotify(env, xsdo);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XDrawRect
 * Signature: (IJIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XDrawRect
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint x, jint y, jint w, jint h)
{
#ifndef HEADLESS
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL || w < 0 || h < 0) {
        return;
    }

    if (w < 2 || h < 2) {
	/* REMIND: This optimization assumes thin lines. */
	/*
	 * This optimization not only simplifies the processing
	 * of a particular degenerate case, but it protects against
	 * the anomalies of various X11 implementations that draw
	 * nothing for degenerate Polygons and Rectangles.
	 */
	XFillRectangle(awt_display, xsdo->drawable, (GC) xgc, 
		       CLAMP_TO_SHORT(x),  CLAMP_TO_SHORT(y),
		       CLAMP_TO_USHORT(w+1), CLAMP_TO_USHORT(h+1));
    } else {
	XDrawRectangle(awt_display, xsdo->drawable, (GC) xgc, 
		       CLAMP_TO_SHORT(x),  CLAMP_TO_SHORT(y),
		       CLAMP_TO_USHORT(w), CLAMP_TO_USHORT(h));
    }
    X11SD_DirectRenderNotify(env, xsdo);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XDrawRoundRect
 * Signature: (IJIIIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XDrawRoundRect
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint x, jint y, jint w, jint h,
     jint arcW, jint arcH)
{
#ifndef HEADLESS
    long ty1, ty2, tx1, tx2, cx, cy, cxw, cyh,
         halfW, halfH, leftW, rightW, topH, bottomH;
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL || w < 0 || h < 0) {
        return;
    }

    arcW = ABS(arcW);
    arcH = ABS(arcH);
    if (arcW > w) {
        arcW = w;
    }
    if (arcH > h) {
        arcH = h;
    }

    if (arcW == 0 || arcH == 0) {
        Java_sun_awt_X11Renderer_XDrawRect(env, xr, pXSData, xgc, x, y, w, h);
        return;
    }

    halfW = (arcW / 2);
    halfH = (arcH / 2);

    /* clamp to short bounding box of round rectangle */
    cx = CLAMP_TO_SHORT(x);
    cy = CLAMP_TO_SHORT(y);
    cxw = CLAMP_TO_SHORT(x + w);
    cyh = CLAMP_TO_SHORT(y + h);

    /* clamp to short coordinates of lines */
    tx1 = CLAMP_TO_SHORT(x + halfW + 1);
    tx2 = CLAMP_TO_SHORT(x + w - halfW - 1);
    ty1 = CLAMP_TO_SHORT(y + halfH + 1);
    ty2 = CLAMP_TO_SHORT(y + h - halfH - 1);
    
    /*
     * recalculate heightes and widthes of round parts
     * to minimize distortions in visible area
     */
    leftW = (tx1 - cx) * 2;
    rightW = (cxw - tx2) * 2;
    topH = (ty1 - cy) * 2;
    bottomH = (cyh - ty2) * 2;

    awt_drawArc(env, xsdo->drawable, (GC) xgc,
                cx, cy, leftW, topH,
                90, 90, JNI_FALSE);
    awt_drawArc(env, xsdo->drawable, (GC) xgc,
                cxw - rightW, cy, rightW, topH,
                0, 90, JNI_FALSE);
    awt_drawArc(env, xsdo->drawable, (GC) xgc,
                cx, cyh - bottomH, leftW, bottomH,
                180, 90, JNI_FALSE);
    awt_drawArc(env, xsdo->drawable, (GC) xgc,
                cxw - rightW, cyh - bottomH, rightW, bottomH,
                270, 90, JNI_FALSE);

    if (tx1 <= tx2) {
        XDrawLine(awt_display, xsdo->drawable, (GC) xgc,
                  tx1, cy, tx2, cy);
        if (h > 0) {
            XDrawLine(awt_display, xsdo->drawable, (GC) xgc,
                      tx1, cyh, tx2, cyh);
        }
    }
    if (ty1 <= ty2) {
        XDrawLine(awt_display, xsdo->drawable, (GC) xgc,
                  cx, ty1, cx, ty2);
        if (w > 0) {
            XDrawLine(awt_display, xsdo->drawable, (GC) xgc,
                      cxw, ty1, cxw, ty2);
        }
    }
    X11SD_DirectRenderNotify(env, xsdo);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XDrawOval
 * Signature: (IJIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XDrawOval
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint x, jint y, jint w, jint h)
{
#ifndef HEADLESS
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL) {
	return;
    }

    if (w < 2 || h < 2) {
	/*
	 * Fix for 4205762 - 1x1 ovals do not draw on Ultra1, Creator3d
	 * (related to 4411814 on Windows platform)
	 * Really small ovals degenerate to simple rectangles as they
	 * have no curvature or enclosed area.  Use XFillRectangle
	 * for speed and to deal better with degenerate sizes.
	 */
	if (w >= 0 && h >= 0) {
	    XFillRectangle(awt_display, xsdo->drawable, (GC) xgc,
			   x, y, w+1, h+1);
	}
    } else {
	awt_drawArc(env, xsdo->drawable, (GC) xgc,
		    x, y, w, h, 0, 360, JNI_FALSE);
    }
    X11SD_DirectRenderNotify(env, xsdo);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XDrawArc
 * Signature: (IJIIIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XDrawArc
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint x, jint y, jint w, jint h,
     jint angleStart, jint angleExtent)
{
#ifndef HEADLESS
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL) {
	return;
    }

    awt_drawArc(env, xsdo->drawable, (GC) xgc,
		x, y, w, h, angleStart, angleExtent, JNI_FALSE);
    X11SD_DirectRenderNotify(env, xsdo);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XDrawPoly
 * Signature: (IJII[I[IIZ)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XDrawPoly
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint transx, jint transy,
     jintArray xcoordsArray, jintArray ycoordsArray, jint npoints,
     jboolean isclosed)
{
#ifndef HEADLESS
    XPoint pTmp[POLYTEMPSIZE], *points;
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL) {
	return;
    }

    if (JNU_IsNull(env, xcoordsArray) || JNU_IsNull(env, ycoordsArray)) {
        JNU_ThrowNullPointerException(env, "coordinate array");
        return;
    }
    if ((*env)->GetArrayLength(env, ycoordsArray) < npoints ||
	(*env)->GetArrayLength(env, xcoordsArray) < npoints)
    {
        JNU_ThrowArrayIndexOutOfBoundsException(env, "coordinate array");
        return;
    }

    if (npoints < 2) {
	return;
    }

    points = transformPoints(env, xcoordsArray, ycoordsArray, transx, transy,
                             pTmp, (int *)&npoints, isclosed);
    if (points == 0) {
        JNU_ThrowOutOfMemoryError(env, "translated coordinate array");
    } else {
	if (npoints == 2) {
	    /*
	     * Some X11 implementations fail to draw anything for
	     * simple 2 point polygons where the vertices are the
	     * same point even though this violates the X11
	     * specification.  For simplicity we will dispatch all
	     * 2 point polygons through XDrawLine even if they are
	     * non-degenerate as this may invoke less processing
	     * down the line than a Poly primitive anyway.
	     */
	    XDrawLine(awt_display, xsdo->drawable, (GC) xgc,
		      points[0].x, points[0].y,
		      points[1].x, points[1].y);
	} else {
	    XDrawLines(awt_display, xsdo->drawable, (GC) xgc,
		       points, npoints, CoordModeOrigin);
	}
	if (points != pTmp) {
	    free(points);
	}
	X11SD_DirectRenderNotify(env, xsdo);
    }
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XFillRect
 * Signature: (IJIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XFillRect
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint x, jint y, jint w, jint h)
{
#ifndef HEADLESS
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL) {
	return;
    }

    XFillRectangle(awt_display, xsdo->drawable, (GC) xgc, 
		   CLAMP_TO_SHORT(x),  CLAMP_TO_SHORT(y),
		   CLAMP_TO_USHORT(w), CLAMP_TO_USHORT(h));
    X11SD_DirectRenderNotify(env, xsdo);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XFillRoundRect
 * Signature: (IJIIIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XFillRoundRect
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint x, jint y, jint w, jint h,
     jint arcW, jint arcH)
{
#ifndef HEADLESS
    long ty1, ty2, tx1, tx2, cx, cy, cxw, cyh,
         halfW, halfH, leftW, rightW, topH, bottomH;
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL || w <= 0 || h <= 0) {
        return;
    }

    arcW = ABS(arcW);
    arcH = ABS(arcH);
    if (arcW > w) {
        arcW = w;
    }
    if (arcH > h) {
        arcH = h;
    }

    if (arcW == 0 || arcH == 0) {
        Java_sun_awt_X11Renderer_XFillRect(env, xr, pXSData, xgc, x, y, w, h);
        return;
    }

    halfW = (arcW / 2);
    halfH = (arcH / 2);

    /* clamp to short bounding box of round rectangle */
    cx = CLAMP_TO_SHORT(x);
    cy = CLAMP_TO_SHORT(y);
    cxw = CLAMP_TO_SHORT(x + w);
    cyh = CLAMP_TO_SHORT(y + h);

    /* clamp to short coordinates of lines */
    tx1 = CLAMP_TO_SHORT(x + halfW + 1);
    tx2 = CLAMP_TO_SHORT(x + w - halfW - 1);
    ty1 = CLAMP_TO_SHORT(y + halfH + 1);
    ty2 = CLAMP_TO_SHORT(y + h - halfH - 1);

    /*
     * recalculate heightes and widthes of round parts
     * to minimize distortions in visible area
     */
    leftW = (tx1 - cx) * 2;
    rightW = (cxw - tx2) * 2;
    topH = (ty1 - cy) * 2;
    bottomH = (cyh - ty2) * 2;

    awt_drawArc(env, xsdo->drawable, (GC) xgc,
                cx, cy, leftW, topH,
                90, 90, JNI_TRUE);
    awt_drawArc(env, xsdo->drawable, (GC) xgc,
                cxw - rightW, cy, rightW, topH,
                0, 90, JNI_TRUE);
    awt_drawArc(env, xsdo->drawable, (GC) xgc,
                cx, cyh - bottomH, leftW, bottomH,
                180, 90, JNI_TRUE);
    awt_drawArc(env, xsdo->drawable, (GC) xgc,
                cxw - rightW, cyh - bottomH, rightW, bottomH,
                270, 90, JNI_TRUE);

    if (tx1 < tx2) {
        if (cy < ty1) {
            XFillRectangle(awt_display, xsdo->drawable, (GC) xgc,
                           tx1, cy, tx2 - tx1, ty1 - cy);
        }
        if (ty2 < cyh) {
            XFillRectangle(awt_display, xsdo->drawable, (GC) xgc,
                           tx1, ty2, tx2 - tx1, cyh - ty2);
        }
    }
    if (ty1 < ty2) {
        XFillRectangle(awt_display, xsdo->drawable, (GC) xgc,
                       cx, ty1, cxw - cx, ty2 - ty1);
    }
    X11SD_DirectRenderNotify(env, xsdo);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XFillOval
 * Signature: (IJIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XFillOval
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint x, jint y, jint w, jint h)
{
#ifndef HEADLESS
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL) {
	return;
    }

    if (w < 3 || h < 3) {
	/*
	 * Fix for 4205762 - 1x1 ovals do not draw on Ultra1, Creator3d
	 * (related to 4411814 on Windows platform)
	 * Most X11 servers drivers have poor rendering
	 * for thin ellipses and the rendering is most strikingly
	 * different from our theoretical arcs.  Ideally we should
	 * trap all ovals less than some fairly large size and
	 * try to draw aesthetically pleasing ellipses, but that
	 * would require considerably more work to get the corresponding
	 * drawArc variants to match pixel for pixel.
	 * Thin ovals of girth 1 pixel are simple rectangles.
	 * Thin ovals of girth 2 pixels are simple rectangles with
	 * potentially smaller lengths.  Determine the correct length
	 * by calculating .5*.5 + scaledlen*scaledlen == 1.0 which
	 * means that scaledlen is the sqrt(0.75).  Scaledlen is
	 * relative to the true length (w or h) and needs to be
	 * adjusted by half a pixel in different ways for odd or
	 * even lengths.
	 */
#define SQRT_3_4 0.86602540378443864676
	if (w > 2 && h > 1) {
	    int adjw = (int) ((SQRT_3_4 * w - ((w&1)-1)) * 0.5);
	    adjw = adjw * 2 + (w&1);
	    x += (w-adjw)/2;
	    w = adjw;
	} else if (h > 2 && w > 1) {
	    int adjh = (int) ((SQRT_3_4 * h - ((h&1)-1)) * 0.5);
	    adjh = adjh * 2 + (h&1);
	    y += (h-adjh)/2;
	    h = adjh;
	}
#undef SQRT_3_4
	if (w > 0 && h > 0) {
	    XFillRectangle(awt_display, xsdo->drawable, (GC) xgc, x, y, w, h);
	}
    } else {
	awt_drawArc(env, xsdo->drawable, (GC) xgc,
		    x, y, w, h, 0, 360, JNI_TRUE);
    }
    X11SD_DirectRenderNotify(env, xsdo);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XFillArc
 * Signature: (IJIIIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XFillArc
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint x, jint y, jint w, jint h,
     jint angleStart, jint angleExtent)
{
#ifndef HEADLESS
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL) {
	return;
    }

    awt_drawArc(env, xsdo->drawable, (GC) xgc,
		x, y, w, h, angleStart, angleExtent, JNI_TRUE);
    X11SD_DirectRenderNotify(env, xsdo);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XFillPoly
 * Signature: (IJII[I[II)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XFillPoly
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jint transx, jint transy,
     jintArray xcoordsArray, jintArray ycoordsArray, jint npoints)
{
#ifndef HEADLESS
    XPoint pTmp[POLYTEMPSIZE], *points;
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL) {
	return;
    }

    if (JNU_IsNull(env, xcoordsArray) || JNU_IsNull(env, ycoordsArray)) {
        JNU_ThrowNullPointerException(env, "coordinate array");
        return;
    }
    if ((*env)->GetArrayLength(env, ycoordsArray) < npoints ||
	(*env)->GetArrayLength(env, xcoordsArray) < npoints)
    {
        JNU_ThrowArrayIndexOutOfBoundsException(env, "coordinate array");
        return;
    }

    if (npoints < 3) {
	return;
    }

    points = transformPoints(env, xcoordsArray, ycoordsArray, transx, transy,
                             pTmp, (int *)&npoints, JNI_FALSE);
    if (points == 0) {
        JNU_ThrowOutOfMemoryError(env, "translated coordinate array");
    } else {
	if (npoints > 2) {
	    XFillPolygon(awt_display, xsdo->drawable, (GC) xgc,
			 points, npoints, Complex, CoordModeOrigin);
	    X11SD_DirectRenderNotify(env, xsdo);
	}
	if (points != pTmp) {
	    free(points);
	}
    }
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    XFillSpans
 * Signature: (IJLsun/java2d/pipe/SpanIterator;JII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11Renderer_XFillSpans
    (JNIEnv *env, jobject xr,
     jlong pXSData, jlong xgc,
     jobject si, jlong pIterator,
     jint transx, jint transy)
{
#ifndef HEADLESS
    SpanIteratorFuncs *pFuncs = (SpanIteratorFuncs *) jlong_to_ptr(pIterator);
    void *srData;
    jint x, y, w, h;
    jint spanbox[4];
    X11SDOps *xsdo = (X11SDOps *) pXSData;

    if (xsdo == NULL) {
	return;
    }

    if (JNU_IsNull(env, si)) {
	JNU_ThrowNullPointerException(env, "span iterator");
	return;
    }
    if (pFuncs == NULL) {
	JNU_ThrowNullPointerException(env, "native iterator not supplied");
	return;
    }

    srData = (*pFuncs->open)(env, si);
    while ((*pFuncs->nextSpan)(srData, spanbox)) {
	x = spanbox[0] + transx;
	y = spanbox[1] + transy;
	w = spanbox[2] - spanbox[0];
	h = spanbox[3] - spanbox[1];
	XFillRectangle(awt_display, xsdo->drawable, (GC) xgc,
		       CLAMP_TO_SHORT(x),  CLAMP_TO_SHORT(y),
		       CLAMP_TO_USHORT(w), CLAMP_TO_USHORT(h));
    }
    (*pFuncs->close)(env, srData);
    X11SD_DirectRenderNotify(env, xsdo);
#endif /* !HEADLESS */
}

/*
 * Class:     sun_awt_X11Renderer
 * Method:    devCopyArea
 * Signature: (Lsun/java2d/SurfaceData;IIIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_X11Renderer_devCopyArea
    (JNIEnv *env, jobject xr,
     jobject xsd,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height)
{
#ifndef HEADLESS
    X11SDOps *xsdo;
    GC xgc;

    xsdo = X11SurfaceData_GetOps(env, xsd);
    if (xsdo == NULL) {
	return;
    }
    xgc = xsdo->GetGC(env, xsdo, NULL, NULL, xsdo->lastpixel);
    if (xgc == NULL) {
	return;
    }

    XCopyArea(awt_display, xsdo->drawable, xsdo->drawable, xgc,
	      srcx, srcy, width, height, dstx, dsty);

    xsdo->ReleaseGC(env, xsdo, xgc);
#endif /* !HEADLESS */
}
