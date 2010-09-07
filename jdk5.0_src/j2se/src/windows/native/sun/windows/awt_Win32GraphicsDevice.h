/*
 * @(#)awt_Win32GraphicsDevice.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_WIN32GRAPHICSDEVICE_H
#define AWT_WIN32GRAPHICSDEVICE_H

#include "awt.h"
extern "C" {
    #include "img_globals.h"
} // extern "C"
#include "colordata.h"
#include "awt_Palette.h"
#include "awt_MMStub.h"
#include "MTSafeArray.h"
#include "dxCapabilities.h"

class AwtPalette;

class AwtWin32GraphicsDevice : public MTSafeArrayElement {
public:
			    AwtWin32GraphicsDevice(int screen, MTSafeArray *arr);
			    ~AwtWin32GraphicsDevice();
    void		    UpdateDeviceColorState();
    void		    SetGrayness(int grayValue);
    int			    GetGrayness() { return colorData->grayscale; }
    HDC			    GetDC();
    void		    ReleaseDC(HDC hDC);
    jobject		    GetColorModel(JNIEnv *env, 
					  jboolean useDeviceSettings);
    void		    Initialize();
    void		    UpdateDynamicColorModel();
    BOOL		    UpdateSystemPalette();
    unsigned int	    *GetSystemPaletteEntries();
    unsigned char	    *GetSystemInverseLUT();
    void		    SetJavaDevice(JNIEnv *env, jobject objPtr);
    HPALETTE		    SelectPalette(HDC hDC);
    void		    RealizePalette(HDC hDC);
    HPALETTE		    GetPalette();
    ColorData		    *GetColorData() { return cData; }
    int	                    GetBitDepth() { return colorData->bitsperpixel; }
    MHND		    GetMonitor() { return monitor; }
    MONITOR_INFO            *GetMonitorInfo() { return pMonitorInfo; }
    DxCapabilities          *GetDxCaps() { return &dxCaps; }
    int			    GetDeviceIndex() { return screen; }
    void		    Release();
    void		    DisableOffscreenAcceleration();

    static int		    DeviceIndexForWindow(HWND hWnd);
    static jobject	    GetColorModel(JNIEnv *env, jboolean dynamic, 
					  int deviceIndex);
    static HPALETTE	    SelectPalette(HDC hDC, int deviceIndex);
    static void		    RealizePalette(HDC hDC, int deviceIndex);
    static ColorData	    *GetColorData(int deviceIndex);
    static int		    GetGrayness(int deviceIndex);
    static void		    UpdateDynamicColorModel(int deviceIndex);
    static BOOL		    UpdateSystemPalette(int deviceIndex);
    static HPALETTE	    GetPalette(int deviceIndex);
    static MHND		    GetMonitor(int deviceIndex);
    static MONITOR_INFO	    *GetMonitorInfo(int deviceIndex);
    static void             ResetAllMonitorInfo();
    static BOOL		    IsPrimaryPalettized() { return primaryPalettized; }
    static int		    GetDefaultDeviceIndex() { return primaryIndex; }
    static void		    DisableOffscreenAccelerationForDevice(MHND hMonitor);
    static DxCapabilities   *GetDxCapsForDevice(MHND hMonitor);

    static int		    primaryIndex;
    static BOOL		    primaryPalettized;
    static jclass	    indexCMClass;
    static jclass	    wToolkitClass;
    static jfieldID	    dynamicColorModelID;
    static jfieldID	    indexCMrgbID;
    static jfieldID	    indexCMcacheID;
    static jfieldID	    accelerationEnabledID;
    static jmethodID	    paletteChangedMID;

private:
    ImgColorData	    *colorData;
    AwtPalette		    *palette;
    ColorData		    *cData;	// Could be static, but may sometime
					// have per-device info in this structure
    BITMAPINFO		    *gpBitmapInfo;
    int			    screen;
    MHND		    monitor;
    MONITOR_INFO            *pMonitorInfo;
    jobject		    javaDevice;
    MTSafeArray		    *devicesArray;
    DxCapabilities          dxCaps;
};

#endif AWT_WIN32GRAPHICSDEVICE_H
