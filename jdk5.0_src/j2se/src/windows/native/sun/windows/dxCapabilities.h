/*
 * @(#)dxCapabilities.h	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef DX_CAPABILITIES_H
#define DX_CAPABILITIES_H

#include "RegistryKey.h"

#define DD_CREATION            L"ddCreation"
#define DD_SURFACE_CREATION    L"ddSurfaceCreation"
#define D3D_CREATION           L"d3dCreation"
#define D3D_LINES              L"d3dLines"
#define D3D_CLIPPED_LINES      L"d3dClippedLines"
#define D3D_TRANSLUCENCY       L"d3dTranslucency"

class DxCapabilities {
private:
    WCHAR *keyName;
    int ddCreation;
    int ddSurfaceCreation;
    int d3dCreation;
    int d3dLines;
    int d3dClippedLines;
    int d3dTranslucency;
    
public:
	DxCapabilities() { keyName = NULL; }
	~DxCapabilities() { if (keyName) free(keyName); }
    void Initialize(WCHAR *keyName);
    int GetDdCreationCap() { return ddCreation; }
    int GetDdSurfaceCreationCap() { return ddSurfaceCreation; }
    int GetD3dCreationCap() { return d3dCreation; }
    int GetD3dLinesCap() { return d3dLines; }
    int GetD3dClippedLinesCap() { return d3dClippedLines; }
    int GetD3dTranslucencyCap() { return d3dTranslucency; }
    WCHAR *GetDeviceName() { return keyName; }
    
    void SetDdCreationCap(int value);
    void SetDdSurfaceCreationCap(int value);
    void SetD3dCreationCap(int value);
    void SetD3dLinesCap(int value);
    void SetD3dClippedLinesCap(int value);
    void SetD3dTranslucencyCap(int value);
    void PrintCaps();
    
private:
    void SetCap(WCHAR *capName, int value);
};

#endif DX_CAPABILITIES_H
