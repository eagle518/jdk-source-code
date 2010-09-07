/*
 * @(#)dxInit.h	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef DXINIT_H
#define DXINIT_H

#include "Win32SurfaceData.h"
#include "Trace.h"
#include "awt_MMStub.h"
#include "dxCapabilities.h"

// Registry definitions: these values are used to determine whether
// acceleration components are untested, working, or broken, depending
// on the results of testing
#define J2D_ACCEL_KEY_ROOT L"Software\\JavaSoft\\Java2D\\"
#define J2D_ACCEL_DRIVER_SUBKEY L"Drivers\\"
#define J2D_ACCEL_DX_NAME L"DXAcceleration"

void    InitDirectX();

void    GetDeviceKeyName(_DISPLAY_DEVICE *displayDevice, WCHAR *devName);

void    CheckFlags();

void    CheckRegistry();

BOOL	D3DTest(DDrawObjectStruct *tmpDdInstance);

BOOL	DDSetupDevice(DDrawObjectStruct *tmpDdInstance, DxCapabilities *dxCaps);

DDrawObjectStruct *CreateDevice(GUID *lpGUID, HMONITOR hMonitor);

BOOL CALLBACK EnumDeviceCallback(GUID FAR* lpGUID, LPSTR szName, LPSTR szDevice, 
				 LPVOID lParam, HMONITOR hMonitor);

BOOL    DDCreateObject();

#endif DXINIT_H
