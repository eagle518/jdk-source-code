/*
 * @(#)dxInit.cpp	1.16 04/01/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "dxInit.h"
#include "ddrawUtils.h"
#include "d3dTestRaster.h"
#include "RegistryKey.h"
#include "WindowsFlags.h"

/**
 * This file holds the functions that handle the initialization
 * process for DirectX.  This process includes checking the 
 * Windows Registry for information about the system and each display device,
 * running any necessary functionality tests, and storing information
 * out to the registry depending on the test results.
 *
 * In general, startup tests should only have to execute once;
 * they will run the first time we initialize ourselves on a
 * particular display device.  After that, we should just be able
 * to check the registry to see what the results of those tests were
 * and enable/disable DirectX support appropriately.  Startup tests
 * may be re-run in situations where we cannot check the display 
 * device information (it may fail on some OSs) or when the
 * display device we start up on is different from the devices
 * we have tested on before (eg, the user has switched video cards
 * or maybe display depths).  The user may also force the tests to be re-run
 * by using the -Dsun.java2d.accelReset flag.
 */


WCHAR                       *j2dAccelKey;       // Name of java2d root key
WCHAR                       *j2dAccelDriverKey; // Name of j2d per-device key
int                         dxAcceleration;     // dx acceleration ability
                                                // according to the Registry
HINSTANCE	            hLibDDraw = NULL;   // DDraw Library handle
extern DDrawObjectStruct    **ddInstance;
extern CriticalSection	    ddInstanceLock;
extern int		    maxDDDevices;
extern int		    currNumDevices;
extern MTSafeArray          *devices;
extern int                  awt_numScreens;
extern char                 *javaVersion;

#define J2D_D3D_FAILURE 		(0)
#define J2D_D3D_SURFACE_OK 	        (1 << 0)
#define J2D_D3D_LINES_OK 		(1 << 1)
#define J2D_D3D_LINE_CLIPPING_OK 	(1 << 2)



/**
 * Called from AwtWin32GraphicsEnv's initScreens() after it initializes
 * all of the display devices.  This function initializes the global
 * DirectX state as well as the per-device DirectX objects.  This process
 * includes:
 *   - Checking native/Java flags to see what the user wants to manually
 *   enable/disable
 *   - Checking the registry to see if DirectX should be globally disabled
 *   - Enumerating the display devices (this returns unique string IDs
 *   for each display device)
 *   - Checking the registry for each device to see what we have stored
 *   there for this device.
 *   - Enumerate the ddraw devices
 *   - For each ddraw device, match it up with the associated device from
 *   EnumDisplayDevices.
 *   - If no registry entries exist, then run a series of tests using
 *   ddraw and d3d, storing the results in the registry for this device ID
 *   (and possibly color depth - test results may be bpp-specific)
 *   - based on the results of the registry storage or the tests, enable
 *   and disable various ddraw/d3d capabilities as appropriate.
 */
void InitDirectX()
{
    // Check registry state for all display devices
    CheckRegistry();
    
    // Need to prevent multiple initializations of the DX objects/primaries.
    // CriticalSection ensures that this initialization will only happen once,
    // even if multiple threads call into this function at startup time.
    static CriticalSection initLock;
    initLock.Enter();
    static dxInitialized = false;
    if (dxInitialized) {
	initLock.Leave();
	return;
    }
    dxInitialized = true;
    initLock.Leave();

    // Check to make sure ddraw is not disabled globally
    if (useDD) {
	if (dxAcceleration == J2D_ACCEL_UNVERIFIED) {
	    RegistryKey::SetIntValue(j2dAccelKey, J2D_ACCEL_DX_NAME,
				     J2D_ACCEL_TESTING, TRUE);
	}
	hLibDDraw = ::LoadLibrary(TEXT("ddraw.dll"));
	if (!hLibDDraw) {
	    DTRACE_PRINTLN("Could not load library");
	    SetDDEnabledFlag(NULL, FALSE);
	    if (dxAcceleration == J2D_ACCEL_UNVERIFIED) {
		RegistryKey::SetIntValue(j2dAccelKey, J2D_ACCEL_DX_NAME,
					 J2D_ACCEL_FAILURE, TRUE);
	    }
	    return;
	}
	if (dxAcceleration == J2D_ACCEL_UNVERIFIED) {
	    RegistryKey::SetIntValue(j2dAccelKey, J2D_ACCEL_DX_NAME,
				     J2D_ACCEL_SUCCESS, TRUE);
	}
	maxDDDevices = 1;
	ddInstance = (DDrawObjectStruct**)safe_Malloc(maxDDDevices * 
	    sizeof(DDrawObjectStruct*));
	if (!DDCreateObject()) {
	    DTRACE_PRINTLN("Could not create ddraw object");
	    SetDDEnabledFlag(NULL, FALSE);
	}
    }
    
    if (checkRegistry) {
	// diagnostic purposes: iterate through all of the registry
	// settings we have just checked or set and print them out to
	// the console
	printf("Registry Settings:\n");
	RegistryKey::PrintValue(j2dAccelKey, J2D_ACCEL_DX_NAME,
				L"  DxAcceleration");
	// Now check the registry entries for all display devices on the system
	int deviceNum = 0;
	_DISPLAY_DEVICE displayDevice;
	displayDevice.dwSize = sizeof(displayDevice);
	while (EnumDisplayDevices(NULL, deviceNum, & displayDevice, 0) &&
	       deviceNum < 20) // avoid infinite loop with buggy drivers
	{ 
	    DxCapabilities caps;
	    if (displayDevice.dwFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
		// We only care about actual display devices.  Devices without
		// this flag could be virtual devices such as NetMeeting
		AwtWin32GraphicsDevice **devArray =
		    (AwtWin32GraphicsDevice **)devices->Lock();
		for (int i = 0; i < awt_numScreens; ++i) {
		    MONITOR_INFO_EXTENDED *pMonInfo =
			(PMONITOR_INFO_EXTENDED) devArray[i]->GetMonitorInfo();
		    if (wcscmp(pMonInfo->strDevice,
			       displayDevice.strDevName) == 0) {
			// this GraphicsDevice matches this DisplayDevice; check
			// the bit depth and grab the appropriate values from
			// the registry
			int bitDepth = devArray[i]->GetBitDepth();
			WCHAR driverKeyName[2048];
			WCHAR fullKeyName[2048];
			GetDeviceKeyName(&displayDevice, driverKeyName);
			swprintf(fullKeyName, L"%s%s\\%d", j2dAccelDriverKey,
				 driverKeyName, bitDepth);
			printf("  Device\\Depth: %S\\%d\n", 
			       driverKeyName, bitDepth);
			caps.Initialize(fullKeyName);
			caps.PrintCaps();
		    }
		}
		devices->Unlock();
	    }
	    deviceNum++;
	}
    }	
}

/**
 * Utility function that derives a unique name for this display
 * device.  We do this by combining the "name" and "string"
 * fields from the displayDevice structure.  Note that we 
 * remove '\' characters from the dev name; since we're going
 * to use this as a registry key, we do not want all those '\'
 * characters to create extra registry key levels.
 */
void GetDeviceKeyName(_DISPLAY_DEVICE *displayDevice, WCHAR *devName)
{
    WCHAR *strDevName = displayDevice->strDevName;
    int devNameIndex = 0;
    for (size_t i = 0; i < wcslen(strDevName); ++i) {
	if (strDevName[i] != L'\\') {
	    devName[devNameIndex++] = strDevName[i];
	}
    }
    devName[devNameIndex++] = L' ';
    devName[devNameIndex] = L'\0';
    wcscat(devName, displayDevice->strDevString);
}


/**
 * CheckRegistry first queries the registry for whether DirectX
 * should be disabled globally.  Then it enumerates the current
 * display devices and queries the registry for each unique display
 * device, putting the resulting values in the AwtWin32GraphicsDevice
 * array for each appropriate display device.
 */
void CheckRegistry()
{
    if (accelReset) {
	RegistryKey::DeleteKey(j2dAccelKey);
    }
    dxAcceleration = RegistryKey::GetIntValue(j2dAccelKey, J2D_ACCEL_DX_NAME);
    if (dxAcceleration == J2D_ACCEL_TESTING ||
	dxAcceleration == J2D_ACCEL_FAILURE)
    {
	// Disable ddraw if previous testing either crashed or failed
	SetDDEnabledFlag(NULL, FALSE);
	// Without DirectX, there is no point to the rest of the registry checks
	// so just return
	return;
    }
    
    // First, get the list of current display devices
    int deviceNum = 0;  // all display devices (virtual and not)
    int numDesktopDevices = 0;  // actual display devices
    _DISPLAY_DEVICE displayDevice;
    displayDevice.dwSize = sizeof(displayDevice);
    _DISPLAY_DEVICE displayDevices[20];
    while (deviceNum < 20 && // avoid infinite loop with buggy drivers
	   EnumDisplayDevices(NULL, deviceNum, &displayDevice, 0))
    {
	if (displayDevice.dwFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) 
	{
	    // We only care about actual display devices.  Devices without
	    // this flag could be virtual devices such as NetMeeting
	    J2dTraceLn2(J2D_TRACE_VERBOSE, "Display Device %d: %S",
			deviceNum, displayDevice.strDevString);
	    displayDevices[numDesktopDevices] = displayDevice;
	    ++numDesktopDevices;
	}
	deviceNum++;
    }
    // Workaround for platforms that do not have the EnumDisplayDevices function
    // (i.e., NT4): just set up a single device that has the display name that
    // has already been assigned to the first (and only) graphics device.
    if (deviceNum == 0) {
	AwtWin32GraphicsDevice **devArray = 
	    (AwtWin32GraphicsDevice **)devices->Lock();
	MONITOR_INFO_EXTENDED *pMonInfo =
	    (PMONITOR_INFO_EXTENDED) devArray[0]->GetMonitorInfo();
	wcscpy(displayDevices[0].strDevName, pMonInfo->strDevice);
	wcscpy(displayDevices[0].strDevString, L"DefaultDriver");
	numDesktopDevices++;
	devices->Unlock();
    }

    // Now, check the current display devices against the list stored
    // in the registry already.
    // First, we get the current list of devices in the registry
    WCHAR subKeyNames[20][1024];
    int numSubKeys = 0;
    {
	RegistryKey hKey(j2dAccelDriverKey, KEY_ALL_ACCESS);
	DWORD buffSize = 1024;
	DWORD ret;
	while (numSubKeys < 20 &&  // same limit as display devices above
	       ((ret = hKey.EnumerateSubKeys(numSubKeys, subKeyNames[numSubKeys], 
					    &buffSize)) ==
	        ERROR_SUCCESS))
	{
	    ++numSubKeys;
	    buffSize = 1024;
	}
    }
    // Now, compare the display devices to the registry display devices
    BOOL devicesDifferent = FALSE;
    // Check that each display device is in the registry
    // Do this by checking each physical display device to see if it 
    // is also in the registry.  If it is, do the same for the rest of
    // the physical devices.  If any device is not in the registry,
    // then there is a mis-match and we break out of the loop and
    // reset the registry.
    for (int i = 0; i < numDesktopDevices; ++i) {
	// Assume the device is not in the registry until proven otherwise
	devicesDifferent = TRUE;
	WCHAR driverName[2048];
	// Key name consists of (driver string) (driver name)
	// but we must remove the "\" characters from the driver
	// name to avoid creating too many levels
	GetDeviceKeyName(&(displayDevices[i]), driverName);
	for (int j = 0; j < numDesktopDevices; ++j) {
	    if (wcscmp(driverName,
		       subKeyNames[j]) == 0) 
	    {
		// Found a match for this device; time to move on
		devicesDifferent = FALSE;
		break;
	    }
	}
	if (devicesDifferent) {
	    J2dTraceLn1(J2D_TRACE_INFO, "Display device %S not in registry",
			driverName);
	    break;
	}
    }
    // Something was different in the runtime versus the registry; delete
    // the registry entries to force testing and writing the results to
    // the registry
    if (devicesDifferent) {
	for (int i = 0; i < numSubKeys; ++i) {
	    WCHAR driverKeyName[2048];
	    swprintf(driverKeyName, L"%s%s", j2dAccelDriverKey,
		     subKeyNames[i]);
	    J2dTraceLn1(J2D_TRACE_INFO, "Deleting registry key: %S\n", 
			driverKeyName);
	    RegistryKey::DeleteKey(driverKeyName);
	}
    }
    
    // Now that we have the display devices and the registry in a good
    // start state, get or initialize the dx capabilities in the registry
    // for each display device
    for (deviceNum = 0; deviceNum < numDesktopDevices; ++deviceNum) {
	AwtWin32GraphicsDevice **devArray = 
	    (AwtWin32GraphicsDevice **)devices->Lock();
	for (int i = 0; i < awt_numScreens; ++i) {
	    MONITOR_INFO_EXTENDED *pMonInfo = 
		(PMONITOR_INFO_EXTENDED)devArray[i]->GetMonitorInfo();
	    if (wcscmp(pMonInfo->strDevice,
		       displayDevices[deviceNum].strDevName) == 0)
	    {
		// this GraphicsDevice matches this DisplayDevice; check
		// the bit depth and grab the appropriate values from
		// the registry
		int bitDepth = devArray[i]->GetBitDepth();
		WCHAR driverKeyName[2048];
		WCHAR fullKeyName[2048];
		// Key name consists of (driver string) (driver name)
		// but we must remove the "\" characters from the driver
		// name to avoid creating too many levels
		GetDeviceKeyName(&(displayDevices[i]), driverKeyName);
		swprintf(fullKeyName, L"%s%s\\%d", j2dAccelDriverKey,
			 driverKeyName, bitDepth);
		// - query registry for key with strDevString\\depth
		devArray[i]->GetDxCaps()->Initialize(fullKeyName);
	    }
	}
	devices->Unlock();
    }
}


/**
 * Output test raster (produced in D3DTest function).  Utility
 * used in debugging only.  Enable by setting J2D_TRACE_LEVEL=J2D_VERBOSE
 * prior to running application with java_g.  The output from this will
 * be seen only if D3DTest fails.
 */
void TestRasterOutput(byte *rasPtr, int x, int y, int w, int h, 
		      int scanStride, int pixelStride)
{
    for (int traceRow = y; traceRow < h; ++traceRow) {
	byte *tmpRasPtr = rasPtr + traceRow * scanStride;
	for (int traceCol = x; traceCol < w; ++traceCol) {
	    DWORD pixelVal;
	    switch (pixelStride) {
	    case 1:
		pixelVal = *tmpRasPtr;
		break;
	    case 2: 
		pixelVal = *((unsigned short*)tmpRasPtr);
		break;
	    default: 
		pixelVal = *((unsigned int*)tmpRasPtr);
		break;
	    }
	    tmpRasPtr += pixelStride;
	    if (pixelVal) {
		J2dTrace(J2D_TRACE_VERBOSE, "1");
	    } else {
		J2dTrace(J2D_TRACE_VERBOSE, "0");
	    }
	}
	J2dTrace(J2D_TRACE_VERBOSE, "\n");
    }
}

/**
 * Test whether we should enable d3d rendering on this device.
 * This includes checking whether there were problems creating
 * the necessary offscreen surface, problems during any of the
 * rendering calls (Blts and d3d lines) and any rendering artifacts
 * caused by d3d lines.  The rendering artifact tests are
 * performed by checking a pre-rendered test pattern (produced
 * by our software renderer) against that same pattern rendered
 * on this device.  If there are any pixels which differ between
 * the two patterns we disable d3d line rendering on the device.
 * Differences in the test pattern rendering can be caused
 * by different rendering algorithms used by our software
 * renderer and the driver or hardware on this device.  For example,
 * some Intel cards (e.g., i815) are known to use polygon renderers
 * for their lines, which sometimes result in wide lines.  
 * The test pattern is stored in d3dTestRaster.h, which is generated
 * by a Java test program 
 * (src/share/test/java2d/VolatileImage/Lines/LinePattern.java).
 */
int D3DTest(DDrawObjectStruct *tmpDdInstance)
{
    DTRACE_PRINTLN("D3DTest");
    HRESULT dxResult;
    // note the "d3dTestRasterH + 2" value; we are adding an 
    // additional clip test at the bottom of the test raster
    DDrawSurface *lpSurface = 
	tmpDdInstance->ddObject->CreateDDOffScreenSurface(d3dTestRasterW, 
            (d3dTestRasterH + 2), 32/*doesn't matter*/, TRUE, TR_OPAQUE, 
            DDSCAPS_VIDEOMEMORY);
    if (!lpSurface) {
	return J2D_D3D_FAILURE;
    }
    DDBLTFX ddBltFx;
    ddBltFx.dwSize = sizeof(ddBltFx);
    ddBltFx.dwFillColor = 0;
    dxResult = lpSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT,
			      &ddBltFx);
    if (dxResult != D3D_OK) {
	DebugPrintDirectDrawError(dxResult, "D3DTest erasing background");
	delete lpSurface;
	return (J2D_D3D_SURFACE_OK | J2D_D3D_FAILURE);
    }
    int color = 0xffffffff;
    int i;
    int vpLeft = 0;
    int vpTop = 0;
    int vpRight = d3dTestRasterW;
    int vpBottom = d3dTestRasterH;
    for (i = 0; i < d3dNumTestLines * 4; i += 4) {
	static D3DTLVERTEX lineVerts[] = {
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	};
	lineVerts[0].sx = (float)d3dTestLines[i];
	lineVerts[0].sy = (float)d3dTestLines[i + 1];
	lineVerts[1].sx = (float)d3dTestLines[i + 2];
	lineVerts[1].sy = (float)d3dTestLines[i + 3];
	dxResult = lpSurface->DrawLineStrip(lineVerts, 2, TRUE, 
					    vpLeft, vpTop, 
					    vpRight, vpBottom);
	if (dxResult != D3D_OK) {
	    DebugPrintDirectDrawError(dxResult, "D3DTest line drawing");
	    delete lpSurface;
	    return (J2D_D3D_SURFACE_OK | J2D_D3D_FAILURE);
	}
    }
    for (i = 0; i < d3dNumTestRects * 4; i += 4) {
	static D3DTLVERTEX lineVerts[5] = {
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	};
	int x = d3dTestRects[i];
	int y = d3dTestRects[i + 1];
	int width = d3dTestRects[i + 2];
	int height = d3dTestRects[i + 3];
	lineVerts[0].sx = (float)x;
	lineVerts[0].sy = (float)y;
	lineVerts[1].sx = (float)x + width;
	lineVerts[1].sy = (float)y;
	lineVerts[2].sx = (float)x + width;
	lineVerts[2].sy = (float)y + height;
	lineVerts[3].sx = (float)x;
	lineVerts[3].sy = (float)y + height;
	lineVerts[4].sx = (float)x;
	lineVerts[4].sy = (float)y;
	HRESULT dxResult = lpSurface->DrawLineStrip(lineVerts, 5, FALSE,
						    vpLeft, vpTop, 
						    vpRight, vpBottom);
	if (dxResult != D3D_OK) {
	    DebugPrintDirectDrawError(dxResult, "D3DTest rect drawing");
	    delete lpSurface;
	    return (J2D_D3D_SURFACE_OK | J2D_D3D_FAILURE);
	}
    }
    // Now for an additional clipping check
    // Draw the same nearly-horizontal line several times; unclipped
    // at first and then with progressively smaller clip areas.  The 
    // pixels filled should be the same every time.  In particular, there
    // should only be one pixel per column.  A clipping error (which
    // could indicate either integer truncation at the clip bounds or
    // a floating point precision problem for lines; we've seen both
    // on different hardware and with different drivers) results in
    // more than one pixel being filled in y as the line steps down
    // at different points on the line based on different clipping.
    // Ideally, the line should look like this:
    //          xxxxxxxxxxxxxxxxxxxxx
    //                               xxxxxxxxxxxxxxxxx
    // A flawed clipping approach might end up filling pixels like so:
    //          xxxxxxxxxxxxxxxxxxxxxxxxx
    //                               xxxxxxxxxxxxxxxxx
    // To check for errors, lock the surface and check that each y value
    // has only one filled pixel.
    for (int testNum = 0; testNum < 20; ++testNum) {
	int vpLeft = testNum;
	int vpTop = 0;
	int vpRight = d3dTestRasterW;
	int vpBottom = d3dTestRasterH + 2; // viewport must include last 2 rows
    
	static D3DTLVERTEX lineVerts[] = {
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	    D3DTLVERTEX(D3DVECTOR(0, 0, .1f), 1.0f, color, 0, 0, 0),
	};
	lineVerts[0].sx = (float)0;
	lineVerts[0].sy = (float)d3dTestRasterH;
	lineVerts[1].sx = (float)d3dTestRasterW - 1;
	lineVerts[1].sy = (float)d3dTestRasterH + 1;
	dxResult = lpSurface->DrawLineStrip(lineVerts, 2, TRUE, 
					    vpLeft, vpTop, 
					    vpRight, vpBottom);
	if (dxResult != D3D_OK) {
	    DebugPrintDirectDrawError(dxResult, "D3DTest line drawing");
	    delete lpSurface;
	    return (J2D_D3D_SURFACE_OK | J2D_D3D_FAILURE);
	}
    }
    // Now, check the results of the test raster against our d3d drawing
    SurfaceDataRasInfo rasInfo;
    dxResult = lpSurface->Lock(NULL, &rasInfo, DDLOCK_WAIT, NULL);
    if (dxResult != DD_OK) {
	delete lpSurface;
	return (J2D_D3D_SURFACE_OK | J2D_D3D_FAILURE);
    }
    byte *rasPtr = (byte*)rasInfo.rasBase;
    int pixelStride = rasInfo.pixelStride;
    int scanStride = rasInfo.scanStride;
    for (int row = 0; row < d3dTestRasterH; ++row) {
	byte *tmpRasPtr = rasPtr + row * scanStride;
	for (int col = 0; col < d3dTestRasterW; ++col) {
	    DWORD pixelVal;
	    switch (pixelStride) {
	    case 1:
		pixelVal = *tmpRasPtr;
		break;
	    case 2: 
		pixelVal = *((unsigned short*)tmpRasPtr);
		break;
	    default: 
		pixelVal = *((unsigned int*)tmpRasPtr);
		break;
	    }
	    tmpRasPtr += pixelStride;
	    // The test is simple: if the test raster pixel has value 0, then
	    // we expect 0 in the d3d surface.  If the test raster has a nonzero
	    // value, then we expect the d3d surface to have that same value.
	    // All other results represent failure.
	    if ((d3dTestRaster[row][col] == 0 && pixelVal != 0) ||
		(d3dTestRaster[row][col] != 0 && pixelVal == 0))
	    {
		J2dTraceLn3(J2D_TRACE_WARNING, 
			    "d3dlines fail due to value %d at (%d, %d)",
		    	    pixelVal, col, row);
#ifdef DEBUG
		// This section is not necessary, but it might be
		// nice to know why we are failing D3DTest on some
		// systems.  If tracing is enabled, this section will
		// produce an ascii representation of the test pattern,
		// the result on this device, and the pixels that were
		// in error.
		J2dTraceLn(J2D_TRACE_VERBOSE, "TestRaster:");
		TestRasterOutput((byte*)d3dTestRaster, 0, 0, d3dTestRasterW, 
				 d3dTestRasterH, d3dTestRasterW, 1);
		J2dTraceLn(J2D_TRACE_VERBOSE, "D3D Raster:");
		TestRasterOutput(rasPtr, 0, 0, d3dTestRasterW, 
				 d3dTestRasterH, scanStride, pixelStride);
		J2dTraceLn(J2D_TRACE_VERBOSE, "Deltas (x indicates problem pixel):");
		for (int traceRow = 0; traceRow < d3dTestRasterH; ++traceRow) {
		    byte *tmpRasPtr = rasPtr + traceRow * scanStride;
		    for (int traceCol = 0; traceCol < d3dTestRasterW; ++traceCol) {
			DWORD pixelVal;
			switch (pixelStride) {
			case 1:
			    pixelVal = *tmpRasPtr;
			    break;
			case 2: 
			    pixelVal = *((unsigned short*)tmpRasPtr);
			    break;
			default: 
			    pixelVal = *((unsigned int*)tmpRasPtr);
			    break;
			}
			tmpRasPtr += pixelStride;
			if ((d3dTestRaster[traceRow][traceCol] == 0 && 
			     pixelVal != 0) ||
			    (d3dTestRaster[traceRow][traceCol] != 0 && 
			     pixelVal == 0))
			{
			    J2dTrace(J2D_TRACE_VERBOSE, "x");
			} else {
			    J2dTrace(J2D_TRACE_VERBOSE, "-");
			}
		    }
		    J2dTrace(J2D_TRACE_VERBOSE, "\n");
		}
#endif // DEBUG
		lpSurface->Unlock(NULL);
		delete lpSurface;
		return (J2D_D3D_SURFACE_OK | J2D_D3D_FAILURE);
	    }
	}
    }
    // Now check the clipped pixels
    row = d3dTestRasterH;
    byte *currRowPtr = rasPtr + row * scanStride;
    byte *nextRowPtr = rasPtr + (row + 1) * scanStride;
    for (int col = 0; col < d3dTestRasterW; ++col) {
	DWORD currPixelVal, nextPixelVal;
	switch (pixelStride) {
	case 1:
	    currPixelVal = *currRowPtr;
	    nextPixelVal = *nextRowPtr;
	    break;
	case 2: 
	    currPixelVal = *((unsigned short*)currRowPtr);
	    nextPixelVal = *((unsigned short*)nextRowPtr);
	    break;
	default: 
	    currPixelVal = *((unsigned int*)currRowPtr);
	    nextPixelVal = *((unsigned int*)nextRowPtr);
	    break;
	}
	if (currPixelVal == nextPixelVal) {
	    J2dTraceLn1(J2D_TRACE_WARNING, "d3dlines fail clipping at column %d", col);
#ifdef DEBUG
	    // This section is not necessary, but it might be
	    // nice to know why we are failing D3DTest on some
	    // systems.  If tracing is enabled, this section will
	    // produce an ascii representation of the test pattern,
	    // the result on this device, and the pixels that were
	    // in error.
	    J2dTraceLn(J2D_TRACE_VERBOSE, "D3D Raster:");
	    TestRasterOutput(rasPtr, 0, d3dTestRasterH, d3dTestRasterW, 
			     d3dTestRasterH + 2, scanStride, pixelStride);
#endif // DEBUG
	    lpSurface->Unlock(NULL);
	    delete lpSurface;
	    J2dTraceLn(J2D_TRACE_INFO, "D3DTest lines ok, but not clipping");
	    return (J2D_D3D_SURFACE_OK | J2D_D3D_LINES_OK);
	}
	currRowPtr += pixelStride;		
	nextRowPtr += pixelStride;
    }
    // Success: we can render d3d lines on this device
    lpSurface->Unlock(NULL);
    delete lpSurface;
    J2dTraceLn(J2D_TRACE_INFO, "D3DTest lines and clipping ok");
    return (J2D_D3D_SURFACE_OK | J2D_D3D_LINES_OK | J2D_D3D_LINE_CLIPPING_OK);
}

/**
 * Check basic ddraw and d3d surface creation.  Return FALSE if we fail
 * just the basic ddraw surface creation, otherwise return TRUE.  This
 * means that we may fail to create rudimentary d3d objects and still
 * return TRUE; we do not base our usage of d3d on the return value
 * from this function, but rather the disablement of d3d usage that
 * happens internal to this function.
 */
BOOL CheckDdD3dCreationCaps(DDrawObjectStruct *tmpDdInstance,
			    DxCapabilities *dxCaps)
{
    // If we have not yet tested this configuration, test it now
    if (dxCaps->GetDdSurfaceCreationCap() == J2D_ACCEL_UNVERIFIED) {
	// First, create a non-d3d offscreen surface
	dxCaps->SetDdSurfaceCreationCap(J2D_ACCEL_TESTING);
	DDrawSurface *lpSurface = 
	    tmpDdInstance->ddObject->CreateDDOffScreenSurface(1, 1, 
	    32, FALSE, TR_OPAQUE, DDSCAPS_VIDEOMEMORY);
	if (!lpSurface) {
	    // problems creating basic ddraw surface - log it and return FALSE
	    dxCaps->SetDdSurfaceCreationCap(J2D_ACCEL_FAILURE);
	    return FALSE;
	}
	// Success; log it and continue
	dxCaps->SetDdSurfaceCreationCap(J2D_ACCEL_SUCCESS);
	delete lpSurface;
    } else if (dxCaps->GetDdSurfaceCreationCap() != J2D_ACCEL_SUCCESS) {
	// we have tested and failed previously; return FALSE
	return FALSE;
    }
    // DDraw surface creation succeeded; what about d3d?
    if (dxCaps->GetD3dCreationCap() == J2D_ACCEL_UNVERIFIED) {
	// First, check this device against a list of bad d3d devices and
	// disable as necessary
	WCHAR *badDeviceStrings[] = {
	    L"Trident Video Accelerator",
	};
	int numBadDevices = 1;
	WCHAR *dxDeviceName = dxCaps->GetDeviceName();
	for (int i = 0; i < numBadDevices; ++i) {
	    if (wcsstr(dxDeviceName, badDeviceStrings[i]) != NULL) {
		J2dTraceLn1(J2D_TRACE_INFO, 
			    "Suspect video card (%s): disable d3d",
			    badDeviceStrings[i]);
		tmpDdInstance->ddObject->DisableD3D();
		dxCaps->SetD3dCreationCap(J2D_ACCEL_FAILURE);
		// REMIND: For now, we disable d3d for all operations because
		// of one bad d3d device in the system.  This is because we
		// should avoid registering the d3d rendering loops at the
		// Java level since we cannot use d3d at the native level.
		// A real fix would instead understand the difference between
		// a surface that could handle d3d native rendering and one
		// that could not and would use the appropriate rendering loop
		// so that disabling d3d on simply one device would be
		// sufficient.
		// Note that this disable-all approach is okay for now because
		// the single bad device (Trident) that triggers this error
		// is generally found on laptops, where multiple graphics
		// devices are not even possible, so disabling d3d for all
		// devices is equivalent to disabling d3d for this single 
		// device.
		SetD3DEnabledFlag(NULL, FALSE, FALSE);
		return TRUE;
	    }
	}
	// First, create a d3d-capable offscreen surface
	dxCaps->SetD3dCreationCap(J2D_ACCEL_TESTING);
	DDrawSurface *lpSurface = 
	    tmpDdInstance->ddObject->CreateDDOffScreenSurface(1, 1, 
	    32, TRUE, TR_OPAQUE, DDSCAPS_VIDEOMEMORY);
	if (!lpSurface) {
	    // Problems with surface creation; disable the use of d3d
	    // on this device, log the failure, and return
	    tmpDdInstance->ddObject->DisableD3D();
	    dxCaps->SetD3dCreationCap(J2D_ACCEL_FAILURE);
	    return TRUE;
	}
	D3DDeviceContext *d3dContext = 
	    tmpDdInstance->ddObject->CreateD3dContext(lpSurface);
	if (!d3dContext) {
	    // Problems with d3d context creation; disable the use of d3d
	    // on this device, log the failure, and return
	    tmpDdInstance->ddObject->DisableD3D();
	    dxCaps->SetD3dCreationCap(J2D_ACCEL_FAILURE);
	    delete lpSurface;
	    return TRUE;
	}
	// Success; log it and continue
	dxCaps->SetD3dCreationCap(J2D_ACCEL_SUCCESS);
	delete d3dContext;
	delete lpSurface;
    } else if (dxCaps->GetD3dCreationCap() != J2D_ACCEL_SUCCESS) {
	// we have tested and failed previously; return FALSE
	tmpDdInstance->ddObject->DisableD3D();
    }	
    return TRUE;
}

BOOL DDSetupDevice(DDrawObjectStruct *tmpDdInstance, DxCapabilities *dxCaps)
{
    DTRACE_PRINTLN("DDSetupDevice");
    BOOL surfaceBasics = CheckDdD3dCreationCaps(tmpDdInstance, dxCaps);
    if (!surfaceBasics) {
	goto FAILURE;
    }
    // create primary surface. There is one of these per ddraw object
    tmpDdInstance->primary = tmpDdInstance->ddObject->CreateDDPrimarySurface(
        (DWORD)tmpDdInstance->backBufferCount);
    if (!tmpDdInstance->primary) {
	goto FAILURE;
    }
    if (!tmpDdInstance->capsSet) {
	DDCAPS caps;
	tmpDdInstance->ddObject->GetDDCaps(&caps);
	tmpDdInstance->canBlt = (caps.dwCaps & DDCAPS_BLT);
	BOOL canCreateOffscreen = tmpDdInstance->canBlt &&
	    (caps.dwVidMemTotal > 0);
	tmpDdInstance->canCreateD3DSurfaces = forceD3DUsage;
	tmpDdInstance->canDrawD3dLines = forceD3DUsage;
	tmpDdInstance->canClipD3dLines = forceD3DUsage;
	// Only register offscreen creation ok if we can Blt and if there
	// is available video memory.  Otherwise it
	// is useless functionality.  The Barco systems apparently allow
	// offscreen creation but do not allow hardware Blt's
	if (canCreateOffscreen) {
	    // Now that we know we can create a surface, let's test its 
	    // capabilities for D3D
	    if (useD3D && !tmpDdInstance->canDrawD3dLines) {
		if (dxCaps->GetD3dCreationCap() == J2D_ACCEL_SUCCESS &&
		    dxCaps->GetD3dLinesCap() == J2D_ACCEL_UNVERIFIED)
		{
		    dxCaps->SetD3dLinesCap(J2D_ACCEL_TESTING);
		    dxCaps->SetD3dClippedLinesCap(J2D_ACCEL_TESTING);
		    int testResults = D3DTest(tmpDdInstance);
		    dxCaps->SetD3dLinesCap(
			(testResults & J2D_D3D_LINES_OK) ?
			J2D_ACCEL_SUCCESS : J2D_ACCEL_FAILURE);
		    dxCaps->SetD3dClippedLinesCap(
			(testResults & J2D_D3D_LINE_CLIPPING_OK) ?
			J2D_ACCEL_SUCCESS : J2D_ACCEL_FAILURE);
		}
		tmpDdInstance->canCreateD3DSurfaces =
		    (dxCaps->GetD3dCreationCap() == J2D_ACCEL_SUCCESS);
		tmpDdInstance->canDrawD3dLines =
		    (dxCaps->GetD3dLinesCap() == J2D_ACCEL_SUCCESS);
		tmpDdInstance->canClipD3dLines =
		    (dxCaps->GetD3dClippedLinesCap() == J2D_ACCEL_SUCCESS);
	    }
	}
	if ((caps.dwCaps & DDCAPS_NOHARDWARE) || !canCreateOffscreen) {
	    AwtWin32GraphicsDevice::DisableOffscreenAccelerationForDevice(
		tmpDdInstance->hMonitor);
	 if (caps.dwCaps & DDCAPS_NOHARDWARE) {
		// Does not have basic functionality we need; release
		// ddraw instance and return FALSE for this device.
		DTRACE_PRINTLN("Disabling ddraw on device: no hw support\n");
		goto FAILURE;
	    }
	}
	tmpDdInstance->capsSet = TRUE;
    }
    // Do NOT create a clipper in full-screen mode
    if (tmpDdInstance->hwndFullScreen == NULL) {
	if (!tmpDdInstance->clipper) {
	    // May have already created a clipper
	    tmpDdInstance->clipper = tmpDdInstance->ddObject->CreateDDClipper();
	}
	if (tmpDdInstance->clipper != NULL) {
	    if (tmpDdInstance->primary->SetClipper(tmpDdInstance->clipper)
		!= DD_OK) 
	    {
		goto FAILURE;
	    }
	} else {
	    goto FAILURE;
	}
    }
    return TRUE;
    
FAILURE:
    AwtWin32GraphicsDevice::DisableOffscreenAccelerationForDevice(
        tmpDdInstance->hMonitor);
    ddInstanceLock.Enter();
    // Do not release the ddInstance structure here, just flag it
    // as having problems; other threads may currently be using a
    // reference to the structure and we cannot release it out from
    // under them.  It will be released sometime later
    // when all DD resources are released.
    tmpDdInstance->accelerated = FALSE;
    ddInstanceLock.Leave();
    return FALSE;    
}

DDrawObjectStruct *CreateDevice(GUID *lpGUID, HMONITOR hMonitor)
{
    DTRACE_PRINTLN2("CreateDevice: lpGUID, hMon = 0x%x, 0x%x\n", lpGUID,
        hMonitor);
    DDrawObjectStruct *tmpDdInstance = 
	(DDrawObjectStruct*)safe_Calloc(1, sizeof(DDrawObjectStruct));
    tmpDdInstance->valid = TRUE;
    tmpDdInstance->accelerated = TRUE;
    tmpDdInstance->capsSet = FALSE;
    tmpDdInstance->hMonitor = hMonitor;
    tmpDdInstance->hwndFullScreen = NULL;
    tmpDdInstance->backBufferCount = 0;
    tmpDdInstance->context = CONTEXT_NORMAL;
    // Create ddraw object
    DxCapabilities *dxCaps = 
        AwtWin32GraphicsDevice::GetDxCapsForDevice(hMonitor);
    if (dxCaps->GetDdCreationCap() == J2D_ACCEL_UNVERIFIED) {
	dxCaps->SetDdCreationCap(J2D_ACCEL_TESTING);
    } else if (dxCaps->GetDdCreationCap() != J2D_ACCEL_SUCCESS) {
	return NULL;
    }
    tmpDdInstance->ddObject = DDraw::CreateDDrawObject(lpGUID);
    if (dxCaps->GetDdCreationCap() == J2D_ACCEL_TESTING) {
	dxCaps->SetDdCreationCap(tmpDdInstance->ddObject ? J2D_ACCEL_SUCCESS :
				                           J2D_ACCEL_FAILURE);
    }
    if (!tmpDdInstance->ddObject) {
	// REMIND: might want to shut down ddraw (useDD == FALSE?)
	// if this error occurs
	return NULL;
    }
    if (DDSetupDevice(tmpDdInstance, dxCaps)) {
	return tmpDdInstance;
    } else {
	return NULL;
    }
}

BOOL CALLBACK EnumDeviceCallback(GUID FAR* lpGUID, LPSTR szName, LPSTR szDevice, 
				 LPVOID lParam, HMONITOR hMonitor)
{
    if (currNumDevices == maxDDDevices) {
	maxDDDevices *= 2;
	DDrawObjectStruct **tmpDDDevices = 
	    (DDrawObjectStruct**)safe_Malloc(maxDDDevices * 
	    sizeof(DDrawObjectStruct*));
	for (int i = 0; i < currNumDevices; ++i) {
	    tmpDDDevices[i] = ddInstance[i];
	}
	DDrawObjectStruct **oldDDDevices = ddInstance;
	ddInstance = tmpDDDevices;
	free(oldDDDevices);
    }
    if (hMonitor != NULL) {
	DDrawObjectStruct    *tmpDdInstance;
	tmpDdInstance = CreateDevice(lpGUID, hMonitor);
	ddInstance[currNumDevices] = tmpDdInstance;
	DTRACE_PRINTLN2("ddInstance[%d] = 0x%x\n", currNumDevices,
	    tmpDdInstance);
	// Increment currNumDevices on success or failure; a null device
	// is perfectly fine; we may have an unaccelerated device
	// in the midst of our multimon configuration
	currNumDevices++;
    }
    return TRUE;
}

typedef HRESULT (WINAPI *FnDDEnumerateFunc)(LPDDENUMCALLBACK cb,
    LPVOID lpContext);

/**
 * Create the ddraw object and the global
 * ddInstance structure.  Note that we do not take the ddInstanceLock
 * here; we assume that our callers are taking that lock for us.
 */
BOOL DDCreateObject() {
    LPDIRECTDRAWENUMERATEEXA lpDDEnum;
    currNumDevices = 0;
    // Note that we are hardcoding this call to the ANSI version and not
    // using the ANIS-or-UNICODE macro name.  This is because there is
    // apparently a problem with the UNICODE function name not being 
    // implemented under the win98 MSLU.  So we just use the ANSI version
    // on all flavors of Windows instead.
    lpDDEnum = (LPDIRECTDRAWENUMERATEEXA)
        GetProcAddress(hLibDDraw, "DirectDrawEnumerateExA");
    if (lpDDEnum) {
        HRESULT ddResult = (lpDDEnum)(EnumDeviceCallback, 
            NULL, DDENUM_ATTACHEDSECONDARYDEVICES);
        if (ddResult != DD_OK) {
            DebugPrintDirectDrawError(ddResult, "DDEnumerate");
        }
    } 
    if (currNumDevices == 0) {
        // Either there was no ddEnumEx function or there was a problem during 
        // enumeration; just create a device on the primary.
        ddInstance[currNumDevices++] = CreateDevice(NULL, NULL);
    }
    DTRACE_PRINTLN("DDCreateDDObject done");
    return TRUE;
}

