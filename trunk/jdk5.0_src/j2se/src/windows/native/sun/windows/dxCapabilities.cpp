/*
 * @(#)dxCapabilities.cpp	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include "dxCapabilities.h"

/**
 * DxCapabilities encapsulates the DirectX capabilities of a display
 * device.  Typically, we run tests at startup on each display device
 * at the current display depth.  We record the results of those tests
 * in a capabilities object and also record those results in the registry.
 * The next time we run on this display device, we check whether we have
 * already recorded results for this device/depth in the registry and simply
 * use those values instead of re-running the tests.  The results of the
 * tests determine which ddraw/d3d capabilities we enable/disable at runtime.
 */

void DxCapabilities::Initialize(WCHAR *keyName)
{
    this->keyName = (WCHAR*)malloc((wcslen(keyName) + 1) * sizeof(WCHAR));
    wcscpy(this->keyName, keyName);
    RegistryKey regKey(keyName, KEY_READ);
    ddCreation = regKey.GetIntValue(DD_CREATION);
    ddSurfaceCreation = regKey.GetIntValue(DD_SURFACE_CREATION);
    d3dCreation = regKey.GetIntValue(D3D_CREATION);
    d3dLines = regKey.GetIntValue(D3D_LINES);
    d3dClippedLines = regKey.GetIntValue(D3D_CLIPPED_LINES);
    d3dTranslucency = regKey.GetIntValue(D3D_TRANSLUCENCY);
}

WCHAR *StringForValue(int value)
{
    switch (value) {
    case J2D_ACCEL_UNVERIFIED:
	return L"UNVERIFIED";
	break;
    case J2D_ACCEL_TESTING:
	return L"TESTING (may indicate crash during test)";
	break;
    case J2D_ACCEL_FAILURE:
	return L"FAILURE";
	break;
    case J2D_ACCEL_SUCCESS:
	return L"SUCCESS";
	break;
    default:
        return L"UNKNOWN";
	break;
    }
}

/**
 * PrintCaps is here for debugging purposes only
 */
void DxCapabilities::PrintCaps() {
    printf("    %S: %S\n", DD_CREATION, StringForValue(ddCreation));
    printf("    %S: %S\n", DD_SURFACE_CREATION, StringForValue(ddSurfaceCreation));
    printf("    %S: %S\n", D3D_CREATION, StringForValue(d3dCreation));
    printf("    %S: %S\n", D3D_LINES, StringForValue(d3dLines));
    printf("    %S: %S\n", D3D_CLIPPED_LINES, StringForValue(d3dClippedLines));
    printf("    %S: %S\n", D3D_TRANSLUCENCY, StringForValue(d3dTranslucency));
}

void DxCapabilities::SetDdCreationCap(int value) {
    ddCreation = value;
    SetCap(DD_CREATION, value);
}

void DxCapabilities::SetDdSurfaceCreationCap(int value) {
    ddSurfaceCreation = value;
    SetCap(DD_SURFACE_CREATION, value);
}

void DxCapabilities::SetD3dCreationCap(int value) {
    d3dCreation = value;
    SetCap(D3D_CREATION, value);
}

void DxCapabilities::SetD3dLinesCap(int value) {
    d3dLines = value;
    SetCap(D3D_LINES, value);
}

void DxCapabilities::SetD3dClippedLinesCap(int value) {
    d3dClippedLines = value;
    SetCap(D3D_CLIPPED_LINES, value);
}

void DxCapabilities::SetD3dTranslucencyCap(int value) {
    d3dTranslucency = value;
    SetCap(D3D_TRANSLUCENCY, value);
}

void DxCapabilities::SetCap(WCHAR *capName, int value) {
    RegistryKey regKey(keyName, KEY_WRITE);
    regKey.SetIntValue(capName, value, TRUE);
}
