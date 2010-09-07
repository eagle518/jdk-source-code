/*
 * @(#)Win32BackBufferSurfaceData.cpp	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_windows_Win32BackBufferSurfaceData.h"

#include "Win32SurfaceData.h"

#include "awt_Component.h"
#include "debug_trace.h"
#include "ddrawUtils.h"

#include "jni_util.h"

extern "C" void initOSSD_WSDO(JNIEnv* env, Win32SDOps* wsdo, jint width,
    jint height, jint screen, jint transparency);

extern "C" void disposeOSSD_WSDO(JNIEnv* env, Win32SDOps* wsdo);

extern jfieldID localD3dEnabledID;  // declared/initialized in Win32OSSD
extern jfieldID d3dClippingEnabledID;  // declared/initialized in Win32OSSD

static DisposeFunc Win32BBSD_Dispose;

/*
 * Class:     sun_awt_windows_Win32BackBufferSurfaceData
 * Method:    initSurface
 * Signature: (IIILsun/awt/windows/Win32BackBufferSurfaceData;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32BackBufferSurfaceData_initSurface(JNIEnv *env, 
    jobject sData, jint depth, jint width, jint height, jint screen,
    jobject parentData)
{
    Win32SDOps *wsdo = (Win32SDOps *)SurfaceData_GetOps(env, sData);

    DTRACE_PRINTLN("Win32BBSD_initSurface");
    /* Set the correct dispose method */
    wsdo->sdOps.Dispose = Win32BBSD_Dispose;
    initOSSD_WSDO(env, wsdo, width, height, screen, JNI_FALSE);
    if (parentData == NULL) {
        SurfaceData_ThrowInvalidPipeException(env, "Parent data is null");
        return;
    }
    Win32SDOps *wsdo_parent = (Win32SDOps*)SurfaceData_GetOps(env, parentData);
    if (!DDGetAttachedSurface(env, wsdo_parent, wsdo)) {
        SurfaceData_ThrowInvalidPipeException(env, 
            "Can't create attached surface");
    }
    if (!D3DEnabled(wsdo)) {
	// d3d enabled by default for each surface - disable if necessary
	env->SetBooleanField(sData, localD3dEnabledID, JNI_FALSE);
    } else {
	env->SetBooleanField(sData, d3dClippingEnabledID, 
			     (DDINSTANCE_USABLE(wsdo->ddInstance) && 
			      wsdo->ddInstance->canClipD3dLines));
    }

    DTRACE_PRINTLN1("Win32OSSD_initSurface done, lpSurface = 0x%x", 
        wsdo->lpSurface);
}

/*
 * Class:     sun_awt_windows_Win32BackBufferSurfaceData
 * Method:    restoreSurface
 * Signature: (Lsun/awt/windows/Win32BackBufferSurfaceData;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32BackBufferSurfaceData_restoreSurface(JNIEnv *env, 
    jobject sData, jobject parentData)
{
    // Noop: back buffer restoration implicit in primary restore
}

/*
 * Method:    Win32BBSD_Dispose
 */
static void
Win32BBSD_Dispose(JNIEnv *env, SurfaceDataOps *ops)
{
    // ops is assumed non-null as it is checked in SurfaceData_DisposeOps
    Win32SDOps *wsdo = (Win32SDOps*)ops;
    if (wsdo->lpSurface != NULL && !wsdo->surfaceLost) {
	delete wsdo->lpSurface;
	wsdo->lpSurface = NULL;
    }
    disposeOSSD_WSDO(env, wsdo);
}
