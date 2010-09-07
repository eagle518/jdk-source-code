/*
 * @(#)Win32D3DRenderer.cpp	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_windows_Win32D3DRenderer.h"
#include "sun_awt_windows_Win32Renderer.h"
#include "Win32SurfaceData.h"

#include <ddraw.h>
#include "ddrawUtils.h"


/*
 * Class:     sun_awt_windows_Win32D3DRenderer
 * Method:    doDrawLineD3D
 * Signature: (Lsun/java2d/SurfaceData;IIIII)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_windows_Win32D3DRenderer_doDrawLineD3D
    (JNIEnv *env, jobject wr,
     jobject sData,
     jint color,
     jint x1, jint y1, jint x2, jint y2,
     jint clipX1, jint clipY1, jint clipX2, jint clipY2)
{
    Win32SDOps	    *wsdo = Win32SurfaceData_GetOpsNoSetup(env, sData);

    DTRACE_PRINTLN("Win32D3DRenderer_doDrawLineD3D");

    return D3DLine(env, wsdo, x1, y1, x2, y2, clipX1, clipY1, clipX2, clipY2, color);
}


/*
 * Class:     sun_awt_windows_Win32D3DRenderer
 * Method:    doDrawRectD3D
 * Signature: (Lsun/java2d/SurfaceData;IIIII)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_windows_Win32D3DRenderer_doDrawRectD3D
    (JNIEnv *env, jobject wr,
     jobject sData,
     jint color,
     jint x, jint y, jint w, jint h,
     jint clipX1, jint clipY1, jint clipX2, jint clipY2)
{
    Win32SDOps	    *wsdo = Win32SurfaceData_GetOpsNoSetup(env, sData);

    DTRACE_PRINTLN("Win32D3DRenderer_doDrawRectD3D");

    return D3DRect(env, wsdo, x, y, w, h, clipX1, clipY1, clipX2, clipY2, color);
}
