/*
 * @(#)ddrawObject.cpp	1.48 04/01/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

 /**
  * ddrawObject.cpp
  *
  * This file holds classes used to access DirectDraw functionality.
  * There are two main classes here used by the outside world:
  * DDraw and DDrawSurface.  DDraw holds the actual DirectDraw
  * device object, responsible for creating surfaces and doing other
  * device-wide operations.  DDraw also holds a pointer to a D3DContext,
  * which has the d3dObject and shared d3d drawing device for the
  * display device (see d3dObject.cpp).  DDrawSurface holds an individual
  * surface, such as the primary or an offscreen surface.  
  * DDrawSurface also holds a pointer to the device-wide d3dContext
  * because some operations on the surface may actually be 3D methods
  * that need to be forwarded to the 3d drawing device.
  * The DirectDraw object and surfaces are wrapped by DXObject
  * and DXSurface classes in order to be able to generically handle
  * DDraw method calls without the caller having to worry about which
  * version of DirectX we are currently running with.  Currently,
  * DXObject has DX5Object and DX7Object subclasses and,
  * similarly, DXSurface has DX5Surface and DX7Surface subclasses.
  * A picture might help to explain the hierarchy of objects herein:
  *
  *                  DDraw (one per display device)
  *                   field: DXObject *dxObject;
  *                   field: DXSurface *lpPrimary
  *                   field: D3DContext (see d3dObject.cpp)
  *
  *
  *                  DXObject (one per display device)
  *                     /\
  *                   /    \
  *                 /        \
  *       DX5Object            DX7Object (DirectX version-specific subclasses)
  *        field: IDirectDraw2   field: IDirectDraw7 (Actual DirectX objects)
  *
  *
  *                  DDrawSurface (one per offscreen or onscreen surface)
  *                   field: DXSurface (for ddraw operations)
  *                   field: D3DContext (shared device, for d3d operations)
  *
  *
  *                  DXSurface (wrapper for DirectDraw operations)
  *                      /\     
  *                    /    \
  *                  /        \
  *                /           DX7Surface (DirectX version-specific subclasses)
  *              /              field: IDirectDrawSurface7 (DirectX object)
  *      DX5Surface          
  *  field: IDirectDrawSurface5 (DirectX object)
  *
  * The wrapper classes work by using the same method calls as the
  * actual DirectX calls and simply forwarding those calls into the
  * the appropriate DirectX object that they contain.  The reason for
  * the indirection is that the subclasses can thus call into the
  * appropriate interface without the caller having to do that
  * explicitly.  So instead of something like:
  *         if (usingDX7) {
  *             dx7Surface->Lock();
  *         } else {
  *             dx5Surface->Lock();
  *         }
  * the caller can simply call:
  *         dxSurface->Lock();
  * and let the magic of subclassing handle the details of which interface
  * to call, depending on which interface was loaded (and thus which
  * subclass was instantiated).
  * The main difference between actual DirectX method calls and the
  * method calls of these wrapper classes is that we avoid using any
  * structures or parameters that are different between the versions
  * of DirectX that we currently support (DX3/5 and DX7).  For example,
  * Lock takes a DDSURFACEDESC structure for DX5 and a 
  * DDSURFACEDESC2 structure for DX7.
  * For these methods, we pick an appropriate higher-level data
  * structure that can be cast and queried as appropriate at the
  * subclass level (in the Lock example, we pass a 
  * SurfaceDataRasInfo structure, holds the data from the
  * call that we need.
  */

#include "ddrawUtils.h"
#include "ddrawObject.h"
#include "WindowsFlags.h"
#include "java_awt_DisplayMode.h"

extern HINSTANCE		    hLibDDraw; // DDraw Library handle

 
#ifdef DEBUG
void StackTrace() {
    JNIEnv* env;
    jvm->AttachCurrentThread((void**)&env, NULL);
    jclass threadClass = env->FindClass("java/lang/Thread");
    jmethodID dumpStackMID = env->GetStaticMethodID(threadClass, "dumpStack", "()V");
    env->CallStaticVoidMethod(threadClass, dumpStackMID);
}
#endif

/**
 * Class DDrawDisplayMode
 */
DDrawDisplayMode::DDrawDisplayMode() :
	width(0), height(0), bitDepth(0), refreshRate(0) {}
DDrawDisplayMode::DDrawDisplayMode(DDrawDisplayMode& rhs) :
	width(rhs.width), height(rhs.height), bitDepth(rhs.bitDepth),
	refreshRate(rhs.refreshRate) {}
DDrawDisplayMode::DDrawDisplayMode(jint w, jint h, jint b, jint r) :
    width(w), height(h), bitDepth(b), refreshRate(r) {}

DDrawDisplayMode::~DDrawDisplayMode() {}


/**
 * Class DDraw::EnumDisplayModesParam
 */
DXObject::EnumDisplayModesParam::EnumDisplayModesParam(
    DDrawDisplayMode::Callback cb, void* ct) : callback(cb), context(ct) {}

DXObject::EnumDisplayModesParam::~EnumDisplayModesParam() {}

/**
 * DX7Object
 * These classes handle operations specific to the DX7 interfaces
 */
DX7Object::DX7Object(IDirectDraw7 *ddObject) {
    this->ddObject = ddObject;
}

DX7Object::~DX7Object()
{
    DTRACE_PRINTLN1("~DX7Object: ddObject = 0x%x\n", ddObject);
    ddObject->Release();
    ddObject = NULL;
}

HRESULT DX7Object::GetAvailableVidMem(DWORD caps, DWORD *total, 
				      DWORD *free)
{
    DDSCAPS2 ddsCaps;
    memset(&ddsCaps, 0, sizeof(ddsCaps));
    ddsCaps.dwCaps = caps;
    return ddObject->GetAvailableVidMem(&ddsCaps, total, free);
}

HRESULT DX7Object::CreateSurface(DWORD dwCaps, DWORD ddsCaps,
				 LPDDPIXELFORMAT lpPf,
				 int width, int height,
				 DXSurface **lpDDSurface,
				 int numBackBuffers) 
{
    IDirectDrawSurface7 *lpSurface;
    HRESULT ddResult;
    DDSURFACEDESC2 ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = dwCaps;
    ddsd.ddsCaps.dwCaps = ddsCaps;
    ddsd.dwWidth = width;
    ddsd.dwHeight = height;
    ddsd.dwBackBufferCount = numBackBuffers;
    if (lpPf) {
	memcpy(&ddsd.ddpfPixelFormat, lpPf, sizeof(DDPIXELFORMAT));
    }
    ddResult = ddObject->CreateSurface(&ddsd, &lpSurface, NULL);
    if (ddResult != DD_OK) {
	DebugPrintDirectDrawError(ddResult, "DX7Object::CreateSurface");
	return ddResult;
    }
    *lpDDSurface = (DXSurface*)new DX7Surface(lpSurface);
    DTRACE_PRINTLN3("DX7Object:: CreateSurface w, h, dxSurf = %d, %d, 0x%x",
    		    width, height, *lpDDSurface);
    return DD_OK;
}

HRESULT DX7Object::GetDisplayMode(DDrawDisplayMode &dm) 
{
    HRESULT ddResult;
    DDSURFACEDESC2 ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddResult = ddObject->GetDisplayMode(&ddsd);
    dm.width = ddsd.dwWidth;
    dm.height = ddsd.dwHeight;
    dm.bitDepth = ddsd.ddpfPixelFormat.dwRGBBitCount;
    dm.refreshRate = ddsd.dwRefreshRate;
    return ddResult;
}

HRESULT DX7Object::EnumDisplayModes(DDrawDisplayMode *dm, 
				    DDrawDisplayMode::Callback callback,
				    void *context) 
{
    DDSURFACEDESC2 ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    LPDDSURFACEDESC2 pDDSD;
    if (dm == NULL) {
	pDDSD = NULL;
    } else {
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = dm->width;
	ddsd.dwHeight = dm->height;
	if (dm->bitDepth != java_awt_DisplayMode_BIT_DEPTH_MULTI) {
	    ddsd.dwFlags |= DDSD_PIXELFORMAT;
	    ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	    ddsd.ddpfPixelFormat.dwRGBBitCount = dm->bitDepth;
	}
	if (dm->refreshRate != java_awt_DisplayMode_REFRESH_RATE_UNKNOWN) {
	    ddsd.dwFlags |= DDSD_REFRESHRATE;
	    ddsd.dwRefreshRate = dm->refreshRate;
	}
	pDDSD = &ddsd;
    }

    EnumDisplayModesParam param(callback, context);

    HRESULT ddResult;
    ddResult = ddObject->EnumDisplayModes(DDEDM_REFRESHRATES, pDDSD, 
					  &param, EnumCallback);
    return ddResult;
}

D3DObject *DX7Object::CreateD3DObject()
{
    IDirect3D7 *d3dInterface;
    D3DObject *d3dObject = NULL;
    HRESULT ddResult = ddObject->QueryInterface(IID_IDirect3D7, 
    						(void**)&d3dInterface);
    if (d3dInterface && (ddResult == DD_OK)) {
	d3dObject = new D3D7Object(d3dInterface);
    } else {
	// QueryInterface can fail here (Itanium); handle it gracefully
	DebugPrintDirectDrawError(ddResult, "Dx7Object::CreateD3DObject");
    }
    return d3dObject;
}



/**
 * DX5Object
 * These methods perform dx5-specific functionality
 */
DX5Object::DX5Object(IDirectDraw2 *ddObject) 
{
    this->ddObject = ddObject;
}

DX5Object::~DX5Object()
{
    DTRACE_PRINTLN1("~DX5Object: ddObject = 0x%x\n", ddObject);
    ddObject->Release();
    ddObject = NULL;
}

HRESULT DX5Object::GetAvailableVidMem(DWORD caps, DWORD *total, 
				      DWORD *free)
{
    DDSCAPS ddsCaps;
    memset(&ddsCaps, 0, sizeof(ddsCaps));
    ddsCaps.dwCaps = caps;
    return ddObject->GetAvailableVidMem(&ddsCaps, total, free);
}

HRESULT DX5Object::CreateSurface(DWORD dwCaps, DWORD ddsCaps,
				 LPDDPIXELFORMAT lpPf,
				 int width, int height,
				 DXSurface **lpDDSurface,
				 int numBackBuffers) 
{
    IDirectDrawSurface *lpSurface;
    HRESULT ddResult;
    DDSURFACEDESC ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = dwCaps;
    ddsd.ddsCaps.dwCaps = ddsCaps;
    ddsd.dwWidth = width;
    ddsd.dwHeight = height;
    ddsd.dwBackBufferCount = numBackBuffers;
    if (lpPf) {
	memcpy(&ddsd.ddpfPixelFormat, lpPf, sizeof(DDPIXELFORMAT));
    }
    ddResult = ddObject->CreateSurface(&ddsd, &lpSurface, NULL);
    if (ddResult != DD_OK) {
	DebugPrintDirectDrawError(ddResult, "DX5Object::CreateSurface");
	return ddResult;
    }
    *lpDDSurface = new DX5Surface(lpSurface);
    
    return DD_OK;
}

HRESULT DX5Object::GetDisplayMode(DDrawDisplayMode &dm) {
    HRESULT ddResult;
    DDSURFACEDESC ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddResult = ddObject->GetDisplayMode(&ddsd);
    dm.width = ddsd.dwWidth;
    dm.height = ddsd.dwHeight;
    dm.bitDepth = ddsd.ddpfPixelFormat.dwRGBBitCount;
    dm.refreshRate = ddsd.dwRefreshRate;
    return ddResult;
}

HRESULT DX5Object::EnumDisplayModes(DDrawDisplayMode *dm, 
				    DDrawDisplayMode::Callback callback,
				    void *context) 
{
    DDSURFACEDESC ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    LPDDSURFACEDESC pDDSD;
    if (dm == NULL) {
	pDDSD = NULL;
    } else {
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = dm->width;
	ddsd.dwHeight = dm->height;
	if (dm->bitDepth != java_awt_DisplayMode_BIT_DEPTH_MULTI) {
	    ddsd.dwFlags |= DDSD_PIXELFORMAT;
	    ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	    ddsd.ddpfPixelFormat.dwRGBBitCount = dm->bitDepth;
	}
	if (dm->refreshRate != java_awt_DisplayMode_REFRESH_RATE_UNKNOWN) {
	    ddsd.dwFlags |= DDSD_REFRESHRATE;
	    ddsd.dwRefreshRate = dm->refreshRate;
	}
	pDDSD = &ddsd;
    }

    EnumDisplayModesParam param(callback, context);

    HRESULT ddResult;
    ddResult = ddObject->EnumDisplayModes(DDEDM_REFRESHRATES, pDDSD, 
					  &param, EnumCallback);
    return ddResult;
}

D3DObject *DX5Object::CreateD3DObject()
{
    IDirect3D2 *d3dInterface;
    D3DObject *d3dObject = NULL;
    HRESULT ddResult = ddObject->QueryInterface(IID_IDirect3D2, 
    						(void**)&d3dInterface);
    if (d3dInterface && (ddResult == DD_OK)) {
	d3dObject = new D3D5Object(d3dInterface);
    } else {
	// QueryInterface can fail here (Itanium); handle it gracefully
	DebugPrintDirectDrawError(ddResult, "Dx5Object::CreateD3DObject");
    }
    return d3dObject;
}


/**
 * Class DDraw
 */
DDraw::DDraw(DXObject *dxObject) {
    DTRACE_PRINTLN("DDraw::DDraw");
    lpPrimary = NULL;
    d3dContext = NULL;
    deviceUseD3D = useD3D;
    this->dxObject = dxObject;
}

DDraw::~DDraw() {
    DTRACE_PRINTLN("DDraw::~DDraw");
    if (dxObject) {
	delete dxObject;
    }
    if (d3dContext) {
	delete d3dContext;
    }
}

DDraw *DDraw::CreateDDrawObject(GUID *lpGUID) {

    DTRACE_PRINTLN("DDraw::Init");
    HRESULT ddResult;
    DXObject *newDXObject;

    // First, try to create a DX7 object
    FnDDCreateExFunc ddCreateEx = NULL;

    if (getenv("NO_J2D_DX7") == NULL) {
	ddCreateEx = (FnDDCreateExFunc)
	::GetProcAddress(hLibDDraw, "DirectDrawCreateEx");
    }

    if (ddCreateEx) {

	DTRACE_PRINTLN("Using DX7");
	// Success - we are going to use the DX7 interfaces
	// create ddraw object
	IDirectDraw7 	*ddObject;

	ddResult = (*ddCreateEx)(lpGUID, (void**)&ddObject, IID_IDirectDraw7, NULL);
	if (ddResult != DD_OK) {
	    DebugPrintDirectDrawError(ddResult, "DDCreateDD dd create");
	    return NULL;
	}
	ddResult = ddObject->SetCooperativeLevel(NULL, 
						 (DDSCL_NORMAL | 
						  DDSCL_FPUPRESERVE));
	if (ddResult != DD_OK) {
	    DebugPrintDirectDrawError(ddResult,
		"DDraw::Init setting cooperative level");
	    return NULL;
	}
	newDXObject = new DX7Object(ddObject);
    
    } else {
	DTRACE_PRINTLN("Using DX5");
	// No DX7, try for DX3/5
	IDirectDraw 	*ddObject;
	IDirectDraw2 	*dd2Object;

	FnDDCreateFunc ddCreate = (FnDDCreateFunc)
	    ::GetProcAddress(hLibDDraw, "DirectDrawCreate");
	if (!ddCreate) {
	    // REMIND: might want to shut down ddraw (useDD == FALSE?)
	    // if this error occurs
	    DTRACE_PRINTLN("Could not create DDraw");
	    return NULL;
	}
	// create ddraw object
	ddResult = (*ddCreate)(lpGUID, &ddObject, NULL);
	if (ddResult != DD_OK) {
	    DebugPrintDirectDrawError(ddResult, "DDCreateDD dd create");
	    return NULL;
	}
	ddResult = ddObject->QueryInterface(IID_IDirectDraw2,
	    (void**)&dd2Object);
	if (ddResult != S_OK) {
	    // DirectDraw2 was supported in DX3; don't bother
	    // with anything less than this
	    DebugPrintDirectDrawError(ddResult,
		"DDraw::CreateDDrawObject finding dd3 interface");
	    return NULL;
	}

	ddResult = dd2Object->SetCooperativeLevel(NULL, DDSCL_NORMAL);
	if (ddResult != DD_OK) {
	    DebugPrintDirectDrawError(ddResult,
				      "DDraw::CreateDD setting coop level");
	    return NULL;
	}
	newDXObject = new DX5Object(dd2Object);
    }

    return new DDraw(newDXObject);
}

BOOL DDraw::GetDDCaps(LPDDCAPS caps) {
    HRESULT ddResult;

    memset(caps, 0, sizeof(*caps));
    caps->dwSize = sizeof(*caps);
    ddResult = dxObject->GetCaps(caps, NULL);
    if (ddResult != DD_OK) {
	DebugPrintDirectDrawError(ddResult, "GetDDCaps");
	return FALSE;
    }
    return TRUE;
}

HRESULT DDraw::GetDDAvailableVidMem(DWORD *freeMem)
{
    DDrawDisplayMode dm;
    HRESULT ddResult;
    
    ddResult = dxObject->GetAvailableVidMem((DDSCAPS_VIDEOMEMORY |
    					     DDSCAPS_OFFSCREENPLAIN),
    					    NULL, freeMem);
    if (*freeMem == 0 || ddResult != DD_OK) {
	// Need to check it out ourselves: allocate as much as we can
	// and return that amount
	DDSURFACEDESC ddsd;
	ZeroMemory (&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof( ddsd );
	HRESULT ddr = dxObject->GetDisplayMode(dm);
	if (ddr != DD_OK) 
	    DebugPrintDirectDrawError(ddr, "getDispMode");
	int bytesPerPixel = dm.bitDepth;
	static int maxSurfaces = 20;
	DXSurface **lpDDSOffscreenVram = (DXSurface**)
	    safe_Malloc(maxSurfaces*sizeof(DXSurface*));
	DWORD dwFlags = (DWORD)(DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH);
	DWORD ddsCaps = (DWORD)(DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN);
	int size = 1024;
	int numVramSurfaces = 0;
	int bitsAllocated = 0;
	BOOL done = FALSE;
	while (!done) {
	    HRESULT hResult = 
	    	dxObject->CreateSurface(dwFlags, ddsCaps, NULL, size, size,
					&lpDDSOffscreenVram[numVramSurfaces]);
	    if (hResult != DD_OK) {
		if (size > 1) {
		    size >>= 1;
		} else {
		    done = TRUE;
		}
	    } else {
		*freeMem += size * size * bytesPerPixel;
		numVramSurfaces++;
		if (numVramSurfaces == maxSurfaces) {
		    // Need to reallocate surface holder array
		    int newMaxSurfaces = 2 * maxSurfaces;
		    DXSurface **newSurfaceArray = (DXSurface**)
			safe_Malloc(maxSurfaces*sizeof(DXSurface*));
		    for (int i= 0; i < maxSurfaces; ++i) {
			newSurfaceArray[i] = lpDDSOffscreenVram[i];
		    }
		    free(lpDDSOffscreenVram);
		    maxSurfaces = newMaxSurfaces;
		    lpDDSOffscreenVram = newSurfaceArray;
		}
	    }
	}
	// Now release all the surfaces we allocated
	for (int i = 0; i < numVramSurfaces; ++i) {
	    delete lpDDSOffscreenVram[i];
	}
	free(lpDDSOffscreenVram);
    }
    return ddResult;
}


DDrawSurface* DDraw::CreateDDOffScreenSurface(DWORD width, DWORD height,
					      DWORD depth,
					      jboolean d3dCapsDesired,
					      jint transparency,
					      DWORD surfaceTypeCaps) 
{
    HRESULT ddResult;
    DTRACE_PRINTLN("DDraw::CreateOffScreenSurface");

    DXSurface *dxSurface;
    DWORD dwFlags, ddsCaps;
    LPDDPIXELFORMAT texturePixelFormat = NULL;
    BOOL create3DSurface =
	deviceUseD3D &&
	(d3dCapsDesired || (transparency == TR_TRANSLUCENT)) &&
	(surfaceTypeCaps & DDSCAPS_VIDEOMEMORY);


    if (create3DSurface) {
	if (transparency == TR_TRANSLUCENT) {
	    if (!d3dContext) {
		// we needed a texture, but there is no d3d context
		// REMIND: might want to check if the context is
		// 'valid' here, i.e. there's actually a  d3d device
		return NULL;
	    }
	} else {
	    // Can only do d3d rendering on non-texture/vidmem surfaces
	    surfaceTypeCaps |= DDSCAPS_3DDEVICE;
	}
    }

    // Create the offscreen surface
    dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;

    switch (transparency) {
    case TR_TRANSLUCENT: {
	if (!d3dContext) {
	    // Should not get here (d3d-disabled cases should be caught
	    // in Java code), but catch the error just in case
	    J2dTraceLn(J2D_TRACE_ERROR, 
		       "Attempted to create d3d texture with null d3dContext");
	    return NULL;
	}
	texturePixelFormat = d3dContext->GetPixelFormat(depth);
	if (!texturePixelFormat) {
	    return NULL;
	}
	dwFlags |= DDSD_PIXELFORMAT;
	ddsCaps = DDSCAPS_TEXTURE | surfaceTypeCaps;
	break;
    }
    case TR_BITMASK:
	dwFlags |= DDSD_CKSRCBLT;
	/*FALLTHROUGH*/
    case TR_OPAQUE:
	ddsCaps = DDSCAPS_OFFSCREENPLAIN | surfaceTypeCaps;
    }

    DDrawSurface* ret = NULL;
    if (dxObject) {
        ddResult = dxObject->CreateSurface(dwFlags, ddsCaps, texturePixelFormat,
					   width, height, &dxSurface);
	if (ddResult == DD_OK) {
	    ret = new DDrawSurface(dxSurface, d3dContext, width, height);
	} else {
	    DebugPrintDirectDrawError(ddResult, "CreateDDOffScreenSurface");
	}
    }
    
    return ret;
}

DDrawSurface* DDraw::CreateDDPrimarySurface(DWORD backBufferCount) 
{
    HRESULT ddResult;
    DXSurface *dxSurface;
    DWORD dwFlags, ddsCaps;
    LPDIRECTDRAWSURFACE lpSurface(NULL);
    DDSURFACEDESC ddsd;
    LPDIRECT3DDEVICE2 lpSurface3D(NULL);
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(DDSURFACEDESC);

    DTRACE_PRINTLN1("Recreating primary, back buffers=%d", backBufferCount); 
    // create primary surface. There is one of these per ddraw object
    dwFlags = DDSD_CAPS;
    ddsCaps = DDSCAPS_PRIMARYSURFACE;
    if (backBufferCount > 0) {
        dwFlags |= DDSD_BACKBUFFERCOUNT;
        ddsCaps |= (DDSCAPS_FLIP | DDSCAPS_COMPLEX);
    }
    if (deviceUseD3D) {
	ddsCaps |= DDSCAPS_3DDEVICE;
    }
    DDrawSurface* ret;
    if (lpPrimary) {

	lpPrimary->GetExclusiveAccess();
	// REMIND: it looks like we need to release
	// d3d resources associated with this
	// surface prior to releasing the dd surfaces;
	if (d3dContext != NULL) {
	    d3dContext->Release3DDevice();
	}
	ddResult = lpPrimary->ReleaseSurface();
	if (ddResult != DD_OK) {
	    DebugPrintDirectDrawError(ddResult, 
		"DDrawSurface: Release old primary");
	}
	lpPrimary->dxSurface = NULL;
    }
    if (dxObject) {
        ddResult = dxObject->CreateSurface(dwFlags, ddsCaps, &dxSurface,
        				   backBufferCount);
    } else {
        if (lpPrimary) {
            lpPrimary->ReleaseExclusiveAccess();
        }
        return NULL;
    }
    if (ddResult != DD_OK) {
        DebugPrintDirectDrawError(ddResult, "DDraw::CreatePrimarySurface");
        if (lpPrimary) {
            lpPrimary->ReleaseExclusiveAccess();
        }
	return NULL;
    }

    DDrawDisplayMode dm;
    dxObject->GetDisplayMode(dm);
    
    if (lpPrimary) {
	lpPrimary->SetNewSurface(dxSurface, dm.width, dm.height);
	lpPrimary->ReleaseExclusiveAccess();
    } else {
	lpPrimary = new DDrawPrimarySurface(dxSurface, d3dContext,
					    dm.width, dm.height);
    }
    if (deviceUseD3D) {
	if (!d3dContext) {
	    d3dContext = CreateD3dContext(lpPrimary);
	    // Already created primary with NULL d3dContext, so update that
	    // variable here
	    lpPrimary->SetD3dContext(d3dContext);
	}
	else {
	    d3dContext->SetNew3DDevice(dxSurface);
	}
    }
    ret = lpPrimary;

    return ret;
}

D3DDeviceContext *DDraw::CreateD3dContext(DDrawSurface *ddSurface)
{
    DXSurface *dxSurface = ddSurface->GetDXSurface();
    if (deviceUseD3D && dxSurface) {
	D3DObject *d3dObject = dxObject->CreateD3DObject();
	if (d3dObject) {
	    return new D3DDeviceContext(dxSurface, d3dObject);
	}
    }
    return NULL;
}

DDrawClipper* DDraw::CreateDDClipper() {
    HRESULT ddResult;

    LPDIRECTDRAWCLIPPER pClipper;

    ddResult = dxObject->CreateClipper(0, &pClipper);

    if (ddResult != DD_OK) {
        DebugPrintDirectDrawError(ddResult, "DDraw::CreateClipper");
        return NULL;
    }

    return new DDrawClipper(pClipper);
}

BOOL DDraw::GetDDDisplayMode(DDrawDisplayMode& dm) {
    HRESULT ddResult;

    ddResult = dxObject->GetDisplayMode(dm);

    if (ddResult != DD_OK) {
        DebugPrintDirectDrawError(ddResult, "GetDDDisplayMode");
        return FALSE;
    }

    return TRUE;
}

HRESULT DDraw::SetDDDisplayMode(DDrawDisplayMode& dm) {

    DTRACE_PRINTLN4("DDraw::SetDisplayMode %dx%dx%d, %d", dm.width,
        dm.height, dm.bitDepth, dm.refreshRate);

    HRESULT ddResult;
    // Sleep so that someone can't programatically set the display mode
    // multiple times very quickly and accidentally crash the driver
    static DWORD prevTime = 0;
    DWORD currTime = ::GetTickCount();
    DWORD timeDiff = (currTime - prevTime);
    if (timeDiff < 500) {
        ::Sleep(500 - timeDiff);
    }
    prevTime = currTime;

    ddResult = dxObject->SetDisplayMode(dm.width, dm.height, dm.bitDepth, 
    					dm.refreshRate);

    return ddResult;
}

/**
 * Private callback used by EnumDisplayModes
 */
long WINAPI DX7Object::EnumCallback(LPDDSURFACEDESC2 pDDSD,
    void* pContext)
{
    EnumDisplayModesParam* pParam = 
    	(EnumDisplayModesParam*)pContext;
    DDrawDisplayMode::Callback callback = pParam->callback;
    void* context = pParam->context;

    DDrawDisplayMode displayMode(pDDSD->dwWidth, pDDSD->dwHeight,
        pDDSD->ddpfPixelFormat.dwRGBBitCount, pDDSD->dwRefreshRate);
    (*callback)(displayMode, context);
    return DDENUMRET_OK;
}

long WINAPI DX5Object::EnumCallback(LPDDSURFACEDESC pDDSD,
    void* pContext)
{
    EnumDisplayModesParam* pParam = (EnumDisplayModesParam*)pContext;
    DDrawDisplayMode::Callback callback = pParam->callback;
    void* context = pParam->context;

    DDrawDisplayMode displayMode(pDDSD->dwWidth, pDDSD->dwHeight,
        pDDSD->ddpfPixelFormat.dwRGBBitCount, pDDSD->dwRefreshRate);
    (*callback)(displayMode, context);
    return DDENUMRET_OK;
}

BOOL DDraw::EnumDDDisplayModes(DDrawDisplayMode* constraint,
    DDrawDisplayMode::Callback callback, void* context) 
{
    HRESULT ddResult;

    ddResult = dxObject->EnumDisplayModes(constraint, callback, context);

    if (ddResult != DD_OK) {
        DebugPrintDirectDrawError(ddResult, "DDraw::EnumDisplayModes");
        return FALSE;
    }

    return TRUE;
}

BOOL DDraw::RestoreDDDisplayMode() {
    HRESULT ddResult;

    ddResult = dxObject->RestoreDisplayMode();

    if (ddResult != DD_OK) {
	DebugPrintDirectDrawError(ddResult, "DDraw::RestoreDisplayMode");
        return FALSE;
    }
    return TRUE;
}

HRESULT DDraw::SetCooperativeLevel(HWND hwnd, DWORD dwFlags) 
{
    HRESULT ddResult;
    ddResult = dxObject->SetCooperativeLevel(hwnd, dwFlags);
    /* On some hardware (Radeon 7500 and GeForce2), attempting
     * to use the d3d device created prior to running FS|EX
     * may cause a system crash.  A workaround is to restore
     * the primary surface and recreate the
     * 3d device on the restored surface.
     */
    if ((ddResult == DD_OK) && lpPrimary && d3dContext) {
	lpPrimary->GetExclusiveAccess();
	if (lpPrimary->IsLost() != DD_OK) {
	    // Only bother with workaround if the primary has been lost
	    // Note that this call may fail with DDERR_WRONGMODE if 
	    // the surface was created in a different mode, but we
	    // do not want to propagate this (non-fatal) error.
	    HRESULT res = lpPrimary->Restore();
	    if (FAILED(res)) {
		DebugPrintDirectDrawError(res, 
					  "SetCooperativeLevel:"
					  " lpPrimary->Restore()");
	    }
	}
	lpPrimary->ReleaseExclusiveAccess();
    }
    return ddResult;
}


DX7Surface::DX7Surface(IDirectDrawSurface7 *lpSurface)
{
    this->lpSurface = lpSurface;
    this->versionID = VERSION_DX7;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
}

HRESULT DX7Surface::Lock(RECT *lockRect, SurfaceDataRasInfo *pRasInfo, 
    			 DWORD dwFlags, HANDLE hEvent)
{
    HRESULT retValue = lpSurface->Lock(lockRect, &ddsd, dwFlags, hEvent);
    if (pRasInfo) {
	// Someone might call Lock() just to synchronize, in which case
	// they don't care about the result and pass in NULL for pRasInfo
	pRasInfo->pixelStride = ddsd.ddpfPixelFormat.dwRGBBitCount / 8;
	pRasInfo->scanStride = ddsd.lPitch;
	pRasInfo->rasBase = (void *) ddsd.lpSurface;
    }
    return retValue;
}

HRESULT DX7Surface::GetAttachedSurface(DWORD dwCaps, DXSurface **bbSurface)
{
    IDirectDrawSurface7 *lpDDSBack;
    HRESULT retValue;
    DDSCAPS2 ddsCaps;
    memset(&ddsCaps, 0, sizeof(ddsCaps));
    ddsCaps.dwCaps = dwCaps;
    
    retValue = lpSurface->GetAttachedSurface(&ddsCaps, &lpDDSBack);
    if (retValue == DD_OK) {
	*bbSurface = new DX7Surface(lpDDSBack);
    }
    return retValue;
}

HRESULT DX7Surface::SetClipper(DDrawClipper *pClipper) 
{
    // A NULL pClipper is valid; it means we want no clipper
    // on this surface
    IDirectDrawClipper *actualClipper = pClipper ? 
    					pClipper->GetClipper() :
    					NULL;
    // Calling SetClipper(NULL) on a surface that currently does
    // not have a clipper can cause a crash on some devices
    // (e.g., Matrox G400), so only call SetClipper(NULL) if
    // there is currently a non-NULL clipper set on this surface.
    if (actualClipper || clipperSet) {
	clipperSet = (actualClipper != NULL);
        return lpSurface->SetClipper(actualClipper);
    }
    return DD_OK;
}

int DX7Surface::GetSurfaceDepth()
{
    if (lpSurface->GetSurfaceDesc(&ddsd) != DD_OK) {
        // Failure: return 0 as an error indication
        return 0;
    }
    return ddsd.ddpfPixelFormat.dwRGBBitCount;
}

DX5Surface::DX5Surface(IDirectDrawSurface *lpSurface)
{
    this->lpSurface = lpSurface;
    this->versionID = VERSION_DX5;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
}

HRESULT DX5Surface::Lock(RECT *lockRect, SurfaceDataRasInfo *pRasInfo, 
    			 DWORD dwFlags, HANDLE hEvent)
{
    HRESULT retValue = lpSurface->Lock(lockRect, &ddsd, dwFlags, hEvent);
    if (pRasInfo) {
	// Someone might call Lock() just to synchronize, in which case
	// they don't care about the result and pass in NULL for pRasInfo
	pRasInfo->pixelStride = ddsd.ddpfPixelFormat.dwRGBBitCount / 8;
	pRasInfo->scanStride = ddsd.lPitch;
	pRasInfo->rasBase = (void *) ddsd.lpSurface;
    }
    return retValue;
}

HRESULT DX5Surface::GetAttachedSurface(DWORD dwCaps, DXSurface **bbSurface)
{
    IDirectDrawSurface *lpDDSBack;
    HRESULT retValue;
    DDSCAPS ddsCaps;
    memset(&ddsCaps, 0, sizeof(ddsCaps));
    ddsCaps.dwCaps = dwCaps;
    
    retValue = lpSurface->GetAttachedSurface(&ddsCaps, &lpDDSBack);
    if (retValue == DD_OK) {
	*bbSurface = new DX5Surface(lpDDSBack);
    }
    return retValue;
}

HRESULT DX5Surface::SetClipper(DDrawClipper *pClipper) 
{
    // A NULL pClipper is valid; it means we want no clipper
    // on this surface
    IDirectDrawClipper *actualClipper = pClipper ? 
    					pClipper->GetClipper() :
    					NULL;
    // Calling SetClipper(NULL) on a surface that currently does
    // not have a clipper can cause a crash on some devices
    // (e.g., Matrox G400), so only call SetClipper(NULL) if
    // there is currently a non-NULL clipper set on this surface.
    if (actualClipper || clipperSet) {
	clipperSet = (actualClipper != NULL);
        return lpSurface->SetClipper(actualClipper);
    }
    return DD_OK;
}

int DX5Surface::GetSurfaceDepth()
{
    if (lpSurface->GetSurfaceDesc(&ddsd) != DD_OK) {
        // Failure: return 0 as an error indication
        return 0;
    }
    return ddsd.ddpfPixelFormat.dwRGBBitCount;
}


/**
 * Class DDrawSurface
 *
 * This class handles all operations on DirectDraw surfaces.
 * Mostly, it is a wrapper for the standard ddraw operations on
 * surfaces, but it also provides some additional functionality.
 * There is a surfaceLock CriticalSection associated with every
 * DDrawSurface which is used to make each instance MT-safe.
 * In general, ddraw itself is MT-safe, but we need to ensure
 * that accesses to our internal variables are MT-safe as well.
 * For example, we may need to recreate the primary surface
 * (during a display-mode-set operation) or release a ddraw
 * surface (due to a call to GraphicsDevice.flush()). The surfaceLock
 * enables us to do these operations without putting other threads
 * in danger of dereferencing garbage memory.
 *
 * If a surface has been released but other threads are still 
 * using it, most methods simply return DD_OK and the calling
 * thread can go about its business without worrying about the
 * failure.  Some methods (Lock and GetDC) return an error
 * code so that the caller does not base further operations on
 * an unsuccessful lock call.
 */

DDrawSurface::DDrawSurface() 
{
    surfaceLock = new DDCriticalSection(this);
    d3dContext = NULL;
    //d3dObject = NULL;
}

DDrawSurface::DDrawSurface(DXSurface *dxSurface, D3DDeviceContext *d3dContext,
			   int width, int height)
{
    surfaceLock = new DDCriticalSection(this);
    CRITICAL_SECTION_ENTER(*surfaceLock);
    this->dxSurface = dxSurface;
    this->d3dContext = d3dContext;
    this->width = width;
    this->height = height;
    DTRACE_PRINTLN1("Created a DXSurface 0x%x\n", dxSurface);
    CRITICAL_SECTION_LEAVE(*surfaceLock);
}

DDrawSurface::~DDrawSurface() 
{
    ReleaseSurface();
    delete surfaceLock;
}

/**
 * This function can only be called when the caller has exclusive
 * access to the DDrawSurface object.  This is done because some
 * surfaces (e.g., the primary surface) must be released before a
 * new one can be created and the surfaceLock must be held during
 * the entire process so that no other thread can access the
 * lpSurface before the process is complete.
 */
void DDrawSurface::SetNewSurface(DXSurface *dxSurface, int width, int height) 
{
    this->dxSurface = dxSurface;
    this->width = width;
    this->height = height;
}

void DDrawSurface::SetD3dContext(D3DDeviceContext *ctxt) {
    FlushD3DContext(); 
    d3dContext = ctxt; 
}

HRESULT DDrawSurface::ReleaseSurface() {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface) {
        CRITICAL_SECTION_LEAVE(*surfaceLock);
        return DD_OK;
    }
    DTRACE_PRINTLN1("Releasing a DXSurface 0x%x", dxSurface);
    FlushD3DContext();
    HRESULT retValue = dxSurface->Release();
    dxSurface = NULL;
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

HRESULT DDrawSurface::SetClipper(DDrawClipper* pClipper) {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return DD_OK;
    }
    HRESULT retValue = dxSurface->SetClipper(pClipper);
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

HRESULT DDrawSurface::SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return DD_OK;
    }
    HRESULT retValue = dxSurface->SetColorKey(dwFlags, lpDDColorKey);
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

HRESULT DDrawSurface::GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return DDERR_NOCOLORKEY;
    }
    HRESULT retValue = dxSurface->GetColorKey(dwFlags, lpDDColorKey);
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

/**
 * NOTE: This function takes the surfaceLock critical section, but
 * does not release that lock. The 
 * Unlock method for this surface MUST be called before anything
 * else can happen on the surface.  This is necessary to prevent the
 * surface from being released or recreated while it is being used.
 * See also Unlock(), GetDC(), and ReleaseDC().
 */
HRESULT DDrawSurface::Lock(LPRECT lockRect, SurfaceDataRasInfo *pRasInfo,
    DWORD dwFlags, HANDLE hEvent) {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	// Return error here so that caller does not assume
	// lock worked and perform operations on garbage data
	// based on that assumption
	return DDERR_INVALIDOBJECT;
    }

    FlushD3DContext();
    HRESULT retValue = dxSurface->Lock(lockRect, pRasInfo, dwFlags, hEvent);
    if (retValue != DD_OK) {
        // Failure should release CriticalSection: either the lock will
        // be attempted again (e.g., DDERR_SURFACEBUSY) or the lock
        // failed and DDUnlock will not be called.
        CRITICAL_SECTION_LEAVE(*surfaceLock);
    }

    return retValue;
}

HRESULT DDrawSurface::Unlock(LPRECT lockRect) {
    if (!dxSurface) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return DD_OK;
    }
    HRESULT retValue = dxSurface->Unlock(lockRect);
    if (retValue != DD_OK && lockRect) {
	// Strange and undocumented bug using pre-DX7 interface;
	// for some reason unlocking the same rectangle as we
	// locked returns a DDERR_NOTLOCKED error, but unlocking
	// NULL (the entire surface) seems to work instead.  It is
	// as if Lock(&rect) actually performs Lock(NULL) implicitly,
	// thus causing Unlock(&rect) to fail but Unlock(NULL) to
	// succeed.  Trap this error specifically and try the workaround
	// of attempting to unlock the whole surface instead.
	retValue = dxSurface->Unlock(NULL);
    }
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

HRESULT DDrawSurface::Blt(LPRECT destRect, DDrawSurface* pSrc,
    LPRECT srcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) {
    LPDIRECTDRAWSURFACE lpSrc = NULL;
    DXSurface *dxSrcSurface = NULL;
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return DD_OK;
    }
    if (pSrc) {
	pSrc->GetExclusiveAccess();
	dxSrcSurface = pSrc->dxSurface;
	if (!dxSrcSurface || (dxSrcSurface->IsLost() != DD_OK)) {
	    // If no src surface, then surface must have been released
	    // by some other thread.  If src is lost, then we should not
	    // attempt this operation (causes a crash on some framebuffers).
	    // Return SURFACELOST error in IsLost() case to force surface
	    // restoration as necessary.
	    HRESULT retError;
	    if (!dxSrcSurface) {
                retError = DD_OK;
	    } else {
                retError = DDERR_SURFACELOST;
	    }
	    pSrc->ReleaseExclusiveAccess();
	    CRITICAL_SECTION_LEAVE(*surfaceLock);
	    return retError;
	}
    }
    FlushD3DContext();
    HRESULT retValue = dxSurface->Blt(destRect, dxSrcSurface, srcRect, dwFlags,
				      lpDDBltFx);
    if (pSrc) {
	pSrc->ReleaseExclusiveAccess();
    }
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

void DDrawSurface::FlushD3DContext() {
    if (d3dContext) {
	d3dContext->GetExclusiveAccess();
	D3DDevice *lpSurface3D = d3dContext->Get3DDevice();
	if (lpSurface3D) {
	    DX_FUNC(lpSurface3D->ForceEndScene());
	}
	d3dContext->ReleaseExclusiveAccess();
    }
}

HRESULT DDrawSurface::DrawLineStrip(D3DTLVERTEX *lpVerts, int numVerts,
				    BOOL drawLastPixel, int clipX1, int clipY1, 
				    int clipX2, int clipY2)
{
#ifdef DEBUG
    DTRACE_PRINTLN("DDrawSurface::DrawLineStrip");
    DTRACE_PRINTLN4("  clip: %d, %d, %d, %d\n", clipX1, clipY1,
    		    clipX2, clipY2);
    for (int i = 0; i < numVerts; ++i) {
	DTRACE_PRINTLN3("  vert[%d] = %f, %f", i, lpVerts[i].sx, 
		        lpVerts[i].sy);
    }
#endif // DEBUG
    // Constrain w/h that we send into SetRenderTarget to be contained
    // in the dimensions of this surface
    clipX1 = max(clipX1, 0);
    clipY1 = max(clipY1, 0);
    clipX2 = min(clipX2, width);
    clipY2 = min(clipY2, height);
    int clipWidth = clipX2 - clipX1;
    int clipHeight = clipY2 - clipY1;
    if (clipWidth <= 0 || clipHeight <= 0) {
	// Empty clip region; noop
	return DD_OK;
    }
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface || !d3dContext) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return DDERR_GENERIC;
    }
    d3dContext->GetExclusiveAccess();
    HRESULT retValue = DDERR_GENERIC;
    D3DDevice *lpSurface3D = d3dContext->Get3DDevice();
    if (lpSurface3D) {
	DX_FUNC(retValue = 
		d3dContext->SetRenderTarget(dxSurface, clipX1, clipY1,
					    clipWidth, clipHeight));
    }
    if (retValue != D3D_OK) {
	d3dContext->ReleaseExclusiveAccess();
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return retValue;
    }
    BOOL d3dError = FALSE;
    try {
	DX_FUNC(retValue = lpSurface3D->BeginScene());
	if (retValue == D3D_OK) {
	    DX_FUNC(retValue =
		    lpSurface3D->DrawPrimitive(D3DPT_LINESTRIP,
					       lpVerts, numVerts));
	    if ((retValue == D3D_OK) && drawLastPixel) {
		DX_FUNC(retValue = 
			lpSurface3D->DrawPrimitive(D3DPT_POINTLIST,
						   &(lpVerts[numVerts-1]), 1));
	    }
	    // Note that EndScene will be deferred unless there's an error
	    DX_FUNC(retValue = lpSurface3D->EndScene(retValue));
	}
    } catch (...) {
	// Note: this EndScene may be unnecessary if the BeginScene
	// failed, but it is better to err on the side of too many
	// endings (which will simply fail) rather than too few.
	DX_FUNC(retValue = lpSurface3D->EndScene(retValue));
	d3dError = TRUE;	
    }
    if (d3dError && (retValue == D3D_OK)) {
	// Returning an error will cause us to avoid using d3d for
	// this surface again
	retValue = DDERR_GENERIC;
    }
    d3dContext->ReleaseExclusiveAccess();
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

HRESULT DDrawSurface::TextureBlt(RECT *rSrc, RECT *rDst, 
				 DDrawSurface *lpSrc,
				 CompositeInfo *compInfo)
{
    D3DTLVERTEX quadVerts[4];
    HRESULT ddResult = DDERR_GENERIC;

    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface || !d3dContext) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return DDERR_GENERIC;
    }

    d3dContext->GetExclusiveAccess();

    D3DTEXTUREHANDLE d3dTexHandle = 0;
    D3DDevice *lpSurface3D = d3dContext->Get3DDevice();
    if (lpSurface3D) {
	if ((ddResult = lpSrc->IsLost()) != DD_OK) {
	    /**
	     * Some D3D implementations do not throw an error when doing this
	     * operation from a texture that has been lost.  To workaround
	     * this situation, manually check whether the texture is lost
	     * prior to doing an operation with it and return the error
	     * ourselves if the texture needs to be restored.
	     */
	    d3dContext->ReleaseExclusiveAccess();
	    CRITICAL_SECTION_LEAVE(*surfaceLock);
	    return ddResult;
	}
	DX_FUNC(ddResult = 
		d3dContext->SetRenderTarget(dxSurface, 0, 0, width, height));
    }
    if (ddResult != D3D_OK) {
	d3dContext->ReleaseExclusiveAccess();
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return ddResult;
    }

    float tw = (float)lpSrc->GetWidth();
    float th = (float)lpSrc->GetHeight();
    float x0 = ((float)rSrc->left + 0.5f)   / tw;
    float y0 = ((float)rSrc->top + 0.5f)    / th;
    float x1 = ((float)rSrc->right  + 0.5f) / tw;
    float y1 = ((float)rSrc->bottom + 0.5f) / th;

    DWORD alphaColor = (DWORD)((float)0xff * compInfo->details.extraAlpha);
    DWORD pix = (alphaColor << 24) | 0xffffff;

    quadVerts[0] =
	D3DTLVERTEX(D3DVECTOR((float)rDst->left, (float)rDst->top, .1f),
		    1, pix, 0, x0, y0);
    quadVerts[1] =
	D3DTLVERTEX(D3DVECTOR((float)rDst->right, (float)rDst->top, .1f),
		    1, pix, 0, x1, y0);
    quadVerts[2] =
	D3DTLVERTEX(D3DVECTOR((float)rDst->right, (float)rDst->bottom, .1f),
		    1, pix, 0, x1, y1);
    quadVerts[3] =
	D3DTLVERTEX(D3DVECTOR((float)rDst->left, (float)rDst->bottom, .1f),
		    1, pix, 0, x0, y1);

#ifdef DEBUG
    DTRACE_PRINTLN("DDrawSurface::BlitTexture");
    for (int i = 0; i < 3; i++) {
	DTRACE_PRINTLN3("  vert[%d] = %f, %f\n", i, quadVerts[i].sx, 
		        quadVerts[i].sy);
    }
#endif // DEBUG

    BOOL d3dError = FALSE;
    try {
	DX_FUNC(ddResult = lpSurface3D->BeginScene());
	if (ddResult == D3D_OK) {
	    DX_FUNC(ddResult = lpSurface3D->SetupTextureRendering(d3dContext, 
								  lpSrc->dxSurface));
	    if (ddResult == DD_OK) {
		DX_FUNC(ddResult = lpSurface3D->DrawPrimitive(D3DPT_TRIANGLEFAN, 
							      (LPVOID)quadVerts, 4));
	    }
	    DX_FUNC(ddResult = lpSurface3D->UnsetTextureRendering(d3dContext));

	    // Note that EndScene will be deferred unless there's an error
	    DX_FUNC(ddResult = lpSurface3D->EndScene(ddResult));
	}
    } catch (...) {
	// just in case
	DX_FUNC(ddResult = lpSurface3D->EndScene(ddResult));
	d3dError = TRUE;
    }

    if (d3dError && (ddResult == D3D_OK)) {
	// Returning an error will cause us to avoid using d3d for
	// this surface again
	ddResult = DDERR_GENERIC;
    }

    d3dContext->ReleaseExclusiveAccess();
    CRITICAL_SECTION_LEAVE(*surfaceLock);

    return ddResult;
}

HRESULT DDrawSurface::Flip(DDrawSurface* pDest, DWORD dwFlags) {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface) {
        CRITICAL_SECTION_LEAVE(*surfaceLock);
        return DD_OK;
    }
    pDest->GetExclusiveAccess();
    if (!pDest->dxSurface) {
        pDest->ReleaseExclusiveAccess();
        CRITICAL_SECTION_LEAVE(*surfaceLock);
        return DD_OK;
    }
    FlushD3DContext();
    HRESULT retValue = dxSurface->Flip(dwFlags);
    pDest->ReleaseExclusiveAccess();
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

HRESULT DDrawSurface::IsLost() {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return DD_OK;
    }
    HRESULT retValue = dxSurface->IsLost();
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

HRESULT DDrawSurface::Restore() {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return DD_OK;
    }
    FlushD3DContext();
    HRESULT retValue = dxSurface->Restore();
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

/**
 * Returns the bit depth of the ddraw surface 
 */
int DDrawSurface::GetSurfaceDepth() {
    int retValue = 0; // default value; 0 indicates some problem getting depth
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (dxSurface) {
        retValue = dxSurface->GetSurfaceDepth();
    }
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}
    
/**
 * As in Lock(), above, we grab the surfaceLock in this function,
 * but do not release it until ReleaseDC() is called.  This is because
 * these functions must be called as a pair (they take a lock on
 * the surface inside the ddraw runtime) and the surface should not 
 * be released or recreated while the DC is held.  The caveat is that
 * a failure in this method causes us to release the surfaceLock here
 * because we will not (and should not) call ReleaseDC if we are returning
 * NULL from GetDC.
 */
HDC DDrawSurface::GetDC() {
    HDC hDC = (HDC)NULL;
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (!dxSurface) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return NULL;
    }
    FlushD3DContext();
    HRESULT ddResult = dxSurface->GetDC(&hDC);
    if (ddResult != DD_OK) {
        DebugPrintDirectDrawError(ddResult, "GetDC");
	if (hDC) {
	    // Probably cannot reach here; we got an error
	    // but we also got a valid hDC.  Release it and
	    // return NULL.  Note that releasing the DC also
	    // releases the surfaceLock so we do not duplicate
	    // that here
	    ReleaseDC(hDC);
	} else {
	    CRITICAL_SECTION_LEAVE(*surfaceLock);
	}
	return NULL;
    }
    return hDC;
}

HRESULT DDrawSurface::ReleaseDC(HDC hDC) {
    if (!hDC) {
	// We should not get here, but just in case we need to trap this
	// situation and simply noop.  Note that we do not release the
	// surfaceLock because we already released it when we failed to
	// get the HDC in the first place in GetDC
	J2dTraceLn(J2D_TRACE_ERROR, "Error: Null HDC received in ReleaseDC");
	return DD_OK;
    }
    if (!dxSurface) {
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	return DD_OK;
    }
    HRESULT retValue = dxSurface->ReleaseDC(hDC);
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return retValue;
}

/**
 * Class DDrawPrimarySurface
 * This sublcass of DDrawSurface handles primary-specific
 * functionality.  In particular, the primary can have a 
 * back buffer associated with it; DDrawPrimarySurface holds
 * the reference to that shared resource.
 */
DDrawPrimarySurface::DDrawPrimarySurface() : DDrawSurface() 
{
    bbHolder = NULL;
}

DDrawPrimarySurface::DDrawPrimarySurface(DXSurface *dxSurface,
					 D3DDeviceContext *d3dContext,
					 int width, int height) :
		     DDrawSurface(dxSurface, d3dContext, width, height)
{
    bbHolder = NULL;
}

DDrawPrimarySurface::~DDrawPrimarySurface() {
}

HRESULT DDrawPrimarySurface::ReleaseSurface() {
    if (bbHolder) {
	delete bbHolder;
	bbHolder = NULL;
    }
    return DDrawSurface::ReleaseSurface();
}

void DDrawPrimarySurface::SetNewSurface(DXSurface *dxSurface, int width, 
					int height)
{
    if (bbHolder) {
	delete bbHolder;
	bbHolder = NULL;
    }
    DDrawSurface::SetNewSurface(dxSurface, width, height);
}

DDrawSurface* DDrawPrimarySurface::GetDDAttachedSurface(DWORD caps) {
    if (!bbHolder) {
	HRESULT ddResult;
	DWORD dwCaps;
	if (caps == 0) {
	    dwCaps = DDSCAPS_BACKBUFFER;
	} else {
	    dwCaps = caps;
	}

	DXSurface *dxSurfaceBB;

	CRITICAL_SECTION_ENTER(*surfaceLock);
	if (!dxSurface) {
	    CRITICAL_SECTION_LEAVE(*surfaceLock);
	    return NULL;
	}
	ddResult = dxSurface->GetAttachedSurface(dwCaps, &dxSurfaceBB);
	CRITICAL_SECTION_LEAVE(*surfaceLock);
	if (ddResult != DD_OK) {
	    DebugPrintDirectDrawError(ddResult, "GetDDAttachedSurface");
	    return NULL;
	}
	bbHolder = new BackBufferHolder(dxSurfaceBB);
    }
    return new DDrawBackBufferSurface(bbHolder, d3dContext, width, height);
}

void DDrawPrimarySurface::ReleaseBackBuffer() {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    backBuffer = NULL;
    CRITICAL_SECTION_LEAVE(*surfaceLock);
}

/**
 * Primary restoration is different from non-primary because 
 * of the d3dContext object.  There is a bug (4754180) on some
 * configurations (including Radeon and GeForce2) where using
 * the d3dDevice associated with a primary that is either lost
 * or has been restored can crash the system.  The solution is
 * to force a primary restoration at the appropriate time and
 * to recreate the d3d device associated with that primary.
 */
HRESULT DDrawPrimarySurface::Restore() {
    if (d3dContext) {
	// In case primary restoration fails, just null out the
	// d3d device.  It will be recreated later when the primary
	// is recreated.
	FlushD3DContext();
	d3dContext->Release3DDevice();
    }
    HRESULT retValue = DDrawSurface::Restore();
    if (d3dContext && SUCCEEDED(retValue)) {
	d3dContext->SetNew3DDevice(dxSurface);
    }
    return retValue;
}


/**
 * Class DDrawBackBufferSurface
 * This subclass handles functionality that is specific to
 * the back buffer surface.  The back buffer is a different
 * type of surface than a typical ddraw surface; it is
 * created by the primary surface and restored/released
 * implicitly by similar operations on the primary.
 * There is only one back buffer per primary, so each
 * DDrawBackBufferSurface object which refers to that object
 * shares the reference to it.  In order to appropriately
 * share this resource (both avoid creating too many objects
 * and avoid leaking those that we create), we use the
 * BackBufferHolder structure to contain the single ddraw
 * surface and register ourselves with that object.  This 
 * allows us to have multi-threaded access to the back buffer
 * because if it was somehow deleted by another thread while we
 * are still using it, then the reference to our lpSurface will
 * be nulled-out for us and we will noop operations on that
 * surface (instead of crashing due to dereferencing a released
 * resource).
 */
DDrawBackBufferSurface::DDrawBackBufferSurface() : DDrawSurface() {
}

DDrawBackBufferSurface::DDrawBackBufferSurface(BackBufferHolder *holder,
					       D3DDeviceContext *d3dContext, 
					       int width, int height) :
			DDrawSurface(holder->GetBackBufferSurface(),
				     d3dContext, width, height)
{
    CRITICAL_SECTION_ENTER(*surfaceLock);
    // Register ourselves with the back buffer container.
    // This means that we will be updated by that container
    // if the back buffer goes away.
    bbHolder = holder;
    bbHolder->Add(this);
    CRITICAL_SECTION_LEAVE(*surfaceLock);
}

/**
 * This destructor removes us from the list of back buffers
 * that hold pointers to the one true back buffer.  It also 
 * nulls-out references to the ddraw and d3d objects to make
 * sure that our parent class does not attempt to release
 * those objects.
 */
DDrawBackBufferSurface::~DDrawBackBufferSurface() {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    if (bbHolder) {
	// Tell the back buffer container that we are no
	// longer alive; otherwise it will try to update
	// us when the back buffer dies.
	bbHolder->Remove(this);
    }
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    // Note: our parent class destructor also calls ReleaseSurface,
    // but a function called within a destructor calls only the local
    // version, not the overridden version.  So we first call our
    // overridden ReleaseSurface to make sure our variables are nulled-out
    // before calling the superclass which will attempt to release
    // non-null objects.
    ReleaseSurface();
}

/**
 * Note that in this subclass' version of ReleaseSurface
 * we merely null-out the references to our objects.
 * They are shared resources and will be deleted elsewhere.
 */
HRESULT DDrawBackBufferSurface::ReleaseSurface() {
    CRITICAL_SECTION_ENTER(*surfaceLock);
    bbHolder = NULL;
    dxSurface = NULL;
    CRITICAL_SECTION_LEAVE(*surfaceLock);
    return DD_OK;
}

/**
 * Class BackBufferHolder
 * This class holds the real ddraw/d3d back buffer surfaces.
 * It also contains a list of everyone that is currently 
 * sharing those resources.  When the back buffer goes away
 * due to the primary being released or deleted), then
 * we tell everyone on the list that the back buffer is
 * gone (by nulling out their references to that object)
 * and thus avoid dereferencing a released resource.
 */
BackBufferHolder::BackBufferHolder(DXSurface *backBuffer)
{
    this->backBuffer = backBuffer;
    bbList = NULL;
}

/**
 * The back buffer is going away; iterate through our
 * list and tell all of those objects that information.
 * Then go ahead and actually release the back buffer's
 * resources.
 */
BackBufferHolder::~BackBufferHolder()
{
    bbLock.Enter();
    BackBufferList *bbListPtr = bbList;
    while (bbListPtr) {
	bbListPtr->backBuffer->ReleaseSurface();
	BackBufferList *bbTmp = bbListPtr;
	bbListPtr = bbListPtr->next;
	delete bbTmp;
    }
    // Note: don't release the ddraw surface; this is
    // done implicitly through releasing the primary
    //if (backBuffer3D) {
	//backBuffer3D->Release();
    //}
    bbLock.Leave();
}

/**
 * Add a new client to the list of objects sharing the
 * back buffer.
 */
void BackBufferHolder::Add(DDrawBackBufferSurface *surf)
{
    bbLock.Enter();
    BackBufferList *bbTmp = new BackBufferList;
    bbTmp->next = bbList;
    bbTmp->backBuffer = surf;
    bbList = bbTmp;
    bbLock.Leave();
}

/**
 * Remove a client from the sharing list.  This happens when
 * a client is deleted; we need to remove it from the list
 * so that we do not later go to update a defunct client
 * in the ~BackBufferHolder() destructor.
 */
void BackBufferHolder::Remove(DDrawBackBufferSurface *surf)
{
    bbLock.Enter();
    BackBufferList *bbListPtr = bbList;
    BackBufferList *bbListPtrPrev = NULL;
    while (bbListPtr) {
	if (bbListPtr->backBuffer == surf) {	    
	    BackBufferList *bbTmp = bbListPtr;
	    if (!bbListPtrPrev) {
		bbList = bbListPtr->next;
	    } else {
		bbListPtrPrev->next = bbTmp->next;
	    }
	    delete bbTmp;
	    break;
	}
	bbListPtrPrev = bbListPtr;
	bbListPtr = bbListPtr->next;
    }
    bbLock.Leave();
}


/**
 * Class DDrawClipper
 */
DDrawClipper::DDrawClipper(LPDIRECTDRAWCLIPPER clipper) : lpClipper(clipper) {}

DDrawClipper::~DDrawClipper() 
{
    if (lpClipper) {
        lpClipper->Release();
    }
}

HRESULT DDrawClipper::SetHWnd(DWORD dwFlags, HWND hwnd) 
{
    return lpClipper->SetHWnd(dwFlags, hwnd);
}

HRESULT DDrawClipper::GetClipList(LPRECT rect, LPRGNDATA rgnData, 
				  LPDWORD rgnSize) 
{
    return lpClipper->GetClipList(rect, rgnData, rgnSize);
}

LPDIRECTDRAWCLIPPER DDrawClipper::GetClipper() 
{
    return lpClipper;
}

