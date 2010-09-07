/*
 * @(#)ddrawUtils.h	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef DDRAWUTILS_H
#define DDRAWUTILS_H


#include <ddraw.h>
#include <jni.h>
#include <windows.h>
#include "Win32SurfaceData.h"
#include "ddrawObject.h"

/**
 * Direct Draw utility functions
 */

#define DDINSTANCE_USABLE(ddInst) \
    ((ddInst) && (ddInst->valid) && (ddInst->accelerated))
    
void	DDRelease();

void	DDReleaseSurfaceMemory(DDrawSurface *lpSurface);

BOOL    DDCreatePrimary(Win32SDOps *wsdo);

void	DDSync();

BOOL	DDCanCreatePrimary(HMONITOR hMon);

BOOL	DDCanBlt(Win32SDOps *wsdo);

BOOL	DDUseDDraw(Win32SDOps *wsdo);

BOOL	DeviceUseDDraw(HMONITOR hMon);

BOOL    DeviceUseD3D(HMONITOR hMon);

BOOL	D3DEnabled(Win32SDOps *wsdo);

void	DDInvalidateDDInstance(DDrawObjectStruct *ddInst);

void	ReleaseDDInstance(DDrawObjectStruct *ddInst);

BOOL    DDEnterFullScreen(HMONITOR hMon, HWND hwnd, HWND topLevelHwnd);

BOOL    DDExitFullScreen(HMONITOR hMon, HWND hwnd);

BOOL    DDGetDisplayMode(HMONITOR hMon, DDrawDisplayMode& displayMode);

BOOL    DDSetDisplayMode(HMONITOR hMon, DDrawDisplayMode& displayMode);

BOOL    DDEnumDisplayModes(HMONITOR hMon, DDrawDisplayMode* constraint,
                           DDrawDisplayMode::Callback callback, void* context);

BOOL	DDClipCheck(Win32SDOps *wsdo, RECT *operationRect);

BOOL	DDLock(JNIEnv *env, Win32SDOps *wsdo, RECT *lockRect, 
	       SurfaceDataRasInfo *pRasInfo);

void	DDUnlock(JNIEnv *env, Win32SDOps *wsdo);

BOOL	DDColorFill(JNIEnv *env, jobject sData, Win32SDOps *wsdo, 
		    RECT *fillRect, jint color);

BOOL	DDBlt(JNIEnv *env, Win32SDOps *wsdoSrc, Win32SDOps *wsdoDst,
	      RECT *rDst, RECT *rSrc, CompositeInfo *compInfo = NULL);

void    DDSetColorKey(JNIEnv *env, Win32SDOps *wsdo, jint color);

BOOL	D3DLine(JNIEnv *env, Win32SDOps *wsdo, int x1, int y1,
		int x2, int y2, int clipX1, int clipY1, int clipX2, int clipY2, 
		jint color);

BOOL	D3DRect(JNIEnv *env, Win32SDOps *wsdo, int x, int y, 
		int width, int height, int clipX1, int clipY1, 
		int clipX2, int clipY2, jint color);

BOOL    DDFlip(JNIEnv *env, Win32SDOps *src, Win32SDOps *dest);

BOOL	DDRestoreSurface(Win32SDOps *wsdo);

jint	DDGetAvailableMemory(HMONITOR hMon);

BOOL	DDCreateSurface(Win32SDOps *wsdo, jboolean d3dCapsDesired);

BOOL	DDCreateOffScreenSurface(Win32SDOps *wsdo, jboolean d3dCapsDesired, 
				 DDrawObjectStruct *ddInst);

BOOL    DDGetAttachedSurface(JNIEnv *env, Win32SDOps* wsdo_parent, Win32SDOps* wsdo);

void	DDDestroySurface(Win32SDOps *wsdo);

BOOL	DDCanReplaceSurfaces(HWND hwnd);

BOOL    DDSurfaceDepthsCompatible(int javaDepth, int nativeDepth);

void	PrintDirectDrawError(DWORD errNum, char *message);

void	DebugPrintDirectDrawError(DWORD errNum, char *message);

void	GetDDErrorString(DWORD errNum, char *buffer);


#endif DDRAWUTILS_H
