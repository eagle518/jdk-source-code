/*
 * @(#)Win32DDRenderer.cpp	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_windows_Win32DDRenderer.h"
#include "Win32SurfaceData.h"

#include <ddraw.h>
#include "ddrawUtils.h"


/*
 * Class:     sun_awt_windows_Win32DDRenderer
 * Method:    doDrawLineDD
 * Signature: (Lsun/java2d/SurfaceData;IIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32DDRenderer_doDrawLineDD
    (JNIEnv *env, jobject wr,
     jobject sData,
     jint color,
     jint x1, jint y1, jint x2, jint y2)
{
    Win32SDOps	    *wsdo = Win32SurfaceData_GetOpsNoSetup(env, sData);
    RECT	    fillRect;

    DTRACE_PRINTLN("Win32DDRenderer::doDrawLineDD");

    // Assume x1 <= x2 and y1 <= y2 (that's the way the
    // Java code is written)
    fillRect.left = x1;
    fillRect.top = y1;
    fillRect.right = x2+1;
    fillRect.bottom = y2+1;
    DDColorFill(env, sData, wsdo, &fillRect, color);
}


/*
 * Class:     sun_awt_windows_Win32Renderer
 * Method:    doFillRect
 * Signature: (Lsun/java2d/SurfaceData;IIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32DDRenderer_doFillRectDD
    (JNIEnv *env, jobject wr,
     jobject sData,
     jint color,
     jint left, jint top, jint right, jint bottom)
{
    Win32SDOps	    *wsdo = Win32SurfaceData_GetOpsNoSetup(env, sData);
    RECT	    fillRect;
    
    DTRACE_PRINTLN("Win32DDRenderer::doFillRectDD");

    fillRect.left = left;
    fillRect.top = top;
    fillRect.right = right;
    fillRect.bottom = bottom;
    DDColorFill(env, sData, wsdo, &fillRect, color);
}


/*
 * Class:     sun_awt_windows_Win32DDRenderer
 * Method:    doDrawRectDD
 * Signature: (Lsun/java2d/SurfaceData;IIIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32DDRenderer_doDrawRectDD
    (JNIEnv *env, jobject wr,
     jobject sData,
     jint color,
     jint x, jint y, jint w, jint h)
{
    Win32SDOps	    *wsdo = Win32SurfaceData_GetOpsNoSetup(env, sData);
    RECT	    fillRect;

    DTRACE_PRINTLN("Win32DDRenderer::doDrawRectDD");

    if (w == 0 || h == 0) {
	fillRect.left = x;
	fillRect.top = y;
	fillRect.right = w + 1;
	fillRect.bottom = h + 1;
	DDColorFill(env, sData, wsdo, &fillRect, color);
    }
    else {
	fillRect.left = x;
	fillRect.top = y;
	fillRect.right = x + w + 1;
	fillRect.bottom = y + 1;
	if (!DDColorFill(env, sData, wsdo, &fillRect, color))
	    return;
	fillRect.top = y + 1;
	fillRect.right = x + 1;
	fillRect.bottom = y + h + 1;
	if (!DDColorFill(env, sData, wsdo, &fillRect, color))
	    return;
	fillRect.left = x + 1;
	fillRect.top = y + h;
	fillRect.right = x + w + 1;
	fillRect.bottom = y + h + 1;
	if (!DDColorFill(env, sData, wsdo, &fillRect, color))
	    return;
	fillRect.left = x + w;
	fillRect.top = y + 1;
	fillRect.bottom = y + h;
	if (!DDColorFill(env, sData, wsdo, &fillRect, color))
	    return;
    }
}
