/*
 * @(#)awt_DrawingSurface.cpp	1.38 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#define _JNI_IMPLEMENTATION_
#include "awt_DrawingSurface.h"
#include "WindowsFlags.h"
#include "awt_Component.h"

jclass jawtVImgClass;
jclass jawtComponentClass;
jfieldID jawtPDataID;
jfieldID jawtSDataID;
jfieldID jawtSMgrID;


/* DSI */

jint JAWTDrawingSurfaceInfo::Init(JAWTDrawingSurface* parent)
{
    TRY;

    JNIEnv* env = parent->env;
    jobject target = parent->target;
    if (JNU_IsNull(env, target)) {
        DTRACE_PRINTLN("NULL target");
        return JAWT_LOCK_ERROR;
    }
    HWND newHwnd = AwtComponent::GetHWnd(env, target);
    if (!::IsWindow(newHwnd)) {
        DTRACE_PRINTLN("Bad HWND");
        return JAWT_LOCK_ERROR;
    }
    jint retval = 0;
    platformInfo = this;
    ds = parent;
    bounds.x = env->GetIntField(target, AwtComponent::xID);
    bounds.y = env->GetIntField(target, AwtComponent::yID);
    bounds.width = env->GetIntField(target, AwtComponent::widthID);
    bounds.height = env->GetIntField(target, AwtComponent::heightID);
    if (hwnd != newHwnd) {
        if (hwnd != NULL) {
            ::ReleaseDC(hwnd, hdc);
            retval = JAWT_LOCK_SURFACE_CHANGED;
	}
	hwnd = newHwnd;
        hdc = ::GetDCEx(hwnd, NULL, DCX_CACHE|DCX_CLIPCHILDREN|DCX_CLIPSIBLINGS);
    }
    clipSize = 1;
    clip = &bounds;
    int screen = AwtWin32GraphicsDevice::DeviceIndexForWindow(hwnd);
    hpalette = AwtWin32GraphicsDevice::GetPalette(screen);

    return retval;

    CATCH_BAD_ALLOC_RET(JAWT_LOCK_ERROR);
}

jint JAWTOffscreenDrawingSurfaceInfo::Init(JAWTOffscreenDrawingSurface* parent)
{
    TRY;

    return JAWT_LOCK_ERROR;

    CATCH_BAD_ALLOC_RET(JAWT_LOCK_ERROR);
}

/* Drawing Surface */

JAWTDrawingSurface::JAWTDrawingSurface(JNIEnv* pEnv, jobject rTarget)
{
    TRY_NO_VERIFY;

    env = pEnv;
    target = env->NewGlobalRef(rTarget);
    Lock = LockSurface;
    GetDrawingSurfaceInfo = GetDSI;
    FreeDrawingSurfaceInfo = FreeDSI;
    Unlock = UnlockSurface;
    info.hwnd = NULL;
    info.hdc = NULL;
    info.hpalette = NULL;

    CATCH_BAD_ALLOC;
}

JAWTDrawingSurface::~JAWTDrawingSurface()
{
    TRY_NO_VERIFY;

    env->DeleteGlobalRef(target);

    CATCH_BAD_ALLOC;
}

JAWT_DrawingSurfaceInfo* JNICALL JAWTDrawingSurface::GetDSI
    (JAWT_DrawingSurface* ds)
{
    TRY;

    if (ds == NULL) {
        DTRACE_PRINTLN("Drawing Surface is NULL");
        return NULL;
    }
    JAWTDrawingSurface* pds = static_cast<JAWTDrawingSurface*>(ds);
    return &(pds->info);
    
    CATCH_BAD_ALLOC_RET(NULL);
}

void JNICALL JAWTDrawingSurface::FreeDSI
    (JAWT_DrawingSurfaceInfo* dsi)
{
    TRY_NO_VERIFY;

    DASSERTMSG(dsi != NULL, "Drawing Surface Info is NULL\n");

    JAWTDrawingSurfaceInfo* jdsi = static_cast<JAWTDrawingSurfaceInfo*>(dsi);

    ::ReleaseDC(jdsi->hwnd, jdsi->hdc);

    CATCH_BAD_ALLOC;
}

jint JNICALL JAWTDrawingSurface::LockSurface
    (JAWT_DrawingSurface* ds)
{
    TRY;

    if (ds == NULL) {
        DTRACE_PRINTLN("Drawing Surface is NULL");
        return JAWT_LOCK_ERROR;
    }
    JAWTDrawingSurface* pds = static_cast<JAWTDrawingSurface*>(ds);
    jint val = pds->info.Init(pds);
    if ((val & JAWT_LOCK_ERROR) != 0) {
	return val;
    }
    val = AwtComponent::GetDrawState(pds->info.hwnd);
    AwtComponent::SetDrawState(pds->info.hwnd, 0);
    return val;

    CATCH_BAD_ALLOC_RET(JAWT_LOCK_ERROR);
}

void JNICALL JAWTDrawingSurface::UnlockSurface
    (JAWT_DrawingSurface* ds)
{
    TRY_NO_VERIFY;

    if (ds == NULL) {
        DTRACE_PRINTLN("Drawing Surface is NULL");
        return;
    }
    JAWTDrawingSurface* pds = static_cast<JAWTDrawingSurface*>(ds);

    CATCH_BAD_ALLOC;
}

JAWTOffscreenDrawingSurface::JAWTOffscreenDrawingSurface(JNIEnv* pEnv, 
							 jobject rTarget)
{
    TRY_NO_VERIFY;
    env = pEnv;
    target = env->NewGlobalRef(rTarget);
    Lock = LockSurface;
    GetDrawingSurfaceInfo = GetDSI;
    FreeDrawingSurfaceInfo = FreeDSI;
    Unlock = UnlockSurface;
    info.dxSurface = NULL;
    info.dx7Surface = NULL;

    CATCH_BAD_ALLOC;
}

JAWTOffscreenDrawingSurface::~JAWTOffscreenDrawingSurface()
{
    env->DeleteGlobalRef(target);
}

JAWT_DrawingSurfaceInfo* JNICALL JAWTOffscreenDrawingSurface::GetDSI
    (JAWT_DrawingSurface* ds)
{
    TRY;

    if (ds == NULL) {
        DTRACE_PRINTLN("Drawing Surface is NULL");
        return NULL;
    }
    JAWTOffscreenDrawingSurface* pds = 
    	static_cast<JAWTOffscreenDrawingSurface*>(ds);
    return &(pds->info);
    
    CATCH_BAD_ALLOC_RET(NULL);
}

void JNICALL JAWTOffscreenDrawingSurface::FreeDSI
    (JAWT_DrawingSurfaceInfo* dsi)
{
}

jint JNICALL JAWTOffscreenDrawingSurface::LockSurface
    (JAWT_DrawingSurface* ds)
{
    return JAWT_LOCK_ERROR;
}

void JNICALL JAWTOffscreenDrawingSurface::UnlockSurface
    (JAWT_DrawingSurface* ds)
{
}

/* C exports */

extern "C" JNIEXPORT JAWT_DrawingSurface* JNICALL DSGetDrawingSurface
    (JNIEnv* env, jobject target)
{
    TRY;

    // See if the target component is a java.awt.Component
    if (env->IsInstanceOf(target, jawtComponentClass)) {
	return new JAWTDrawingSurface(env, target);
    }

    DTRACE_PRINTLN("GetDrawingSurface target must be a Component");
    return NULL;

    CATCH_BAD_ALLOC_RET(NULL);
}

extern "C" JNIEXPORT void JNICALL DSFreeDrawingSurface
    (JAWT_DrawingSurface* ds)
{
    TRY_NO_VERIFY;

    if (ds == NULL) {
        DTRACE_PRINTLN("Drawing Surface is NULL");
    }
    delete static_cast<JAWTDrawingSurface*>(ds);

    CATCH_BAD_ALLOC;
}

extern "C" JNIEXPORT void JNICALL DSLockAWT(JNIEnv* env)
{
    // Do nothing on Windows
}

extern "C" JNIEXPORT void JNICALL DSUnlockAWT(JNIEnv* env)
{
    // Do nothing on Windows
}
