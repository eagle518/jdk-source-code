/*
 * @(#)d3dObject.cpp	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
 /**
  * d3dObject.cpp
  *
  * This file holds classes used to access Direct3D functionality.
  * The main interface between this file and the outside world
  * is D3DContext, which holds pointers to both the D3D object
  * and the D3D drawing device for a given display device.
  * The D3D object and device objects are wrapped by D3DObject
  * and D3DDevice classes in order to be able to generically handle
  * D3D method calls without the caller having to worry about which
  * version of DirectX we are currently running with.  Currently,
  * D3DObject has D3D5Object and D3D7Object subclasses and,
  * similarly, D3DDevice has D3D5Device and D3D7Device subclasses.
  * A picture might help to explain the hierarchy of objects herein:
  *
  *                  D3DContext (one per display device)
  *                   field: D3DObject
  *                   field: D3DDevice
  *
  *
  *                  D3DObject (one per display device)
  *                     /\
  *                   /    \
  *                 /        \
  *       D3D5Object          D3D7Object (DirectX version-specific subclasses)
  *        field: IDirect3D2   field: IDirect3D7 (Actual DirectX objects)
  *
  *
  *                  D3DDevice (one per display device, but
  *                      /\     may be recreated upon primary recreation)
  *                    /    \
  *                  /        \
  *                /            \
  *              /                \
  *      D3D5Device          D3D7Device (DirectX version-specific subclasses)
  *  field: IDirect3DDevice2   field: IDirect3DDevice7 (DirectX objects)
  *
  * The wrapper classes work by using the same method calls as the
  * actual DirectX calls and simply forwarding those calls into the
  * the appropriate DirectX object that they contain.  The reason for
  * the indirection is that the subclasses can thus call into the
  * appropriate interface without the caller having to do that
  * explicitly.  So instead of something like:
  *         if (usingDX7) {
  *             d3d7Device->DrawPrimitive();
  *         } else {
  *             d3d5Device->DrawPrimitive();
  *         }
  * the caller can simply call:
  *         d3dDevice->DrawPrimitive();
  * and let the magic of subclassing handle the details of which interface
  * to call, depending on which interface was loaded (and thus which
  * subclass was instantiated).
  * The main difference between actual DirectX method calls and the
  * method calls of these wrapper classes is that we avoid using any
  * structures or parameters that are different between the versions
  * of DirectX that we currently support (DX3/5 and DX7).  For example,
  * SetRenderTarget takes a DirectDraw surface pointer; this type will
  * differ depending on which version of DirectDraw we are using.
  * For these methods, we pick an appropriate higher-level data
  * structure that can be cast and queried as appropriate at the
  * subclass level (in the SetRenderTarget example, we pass a 
  * DXSurface object, which contains the DDraw surface that we need).
  */

#include "ddrawUtils.h"
#include "d3dObject.h"


/**
 * D3D5Object
 */

D3D5Object::~D3D5Object()
{
    DTRACE_PRINTLN1("~D3D5Object: d3dObject = 0x%x\n", d3dObject);
    if (d3dObject) {
	d3dObject->Release();
	d3dObject = NULL;
    }
}

HRESULT D3D5Object::CreateDevice(REFCLSID rclsid, DXSurface *dxSurface,
			     	 D3DDevice **d3dDevice)

{
    DTRACE_PRINTLN2("D3D5Object::CreateDevice: dxSurf, d3dDev = 0x%x, 0x%x",
    		    dxSurface, d3dDevice);
    IDirect3DDevice2 *realD3dDevice;
    HRESULT d3dResult = d3dObject->CreateDevice(rclsid, 
	((DX5Surface*)dxSurface)->GetDDSurface(), &realD3dDevice);
    if (d3dResult == D3D_OK) {
	IDirect3DViewport2 *d3dViewport;
	d3dResult = d3dObject->CreateViewport(&d3dViewport, NULL);
	if (d3dResult == D3D_OK) {
	    *d3dDevice = new D3D5Device(realD3dDevice, d3dViewport);
	}
    }
    return d3dResult;
}

/**
 * D3D7Object
 */
 
D3D7Object::~D3D7Object()
{
    DTRACE_PRINTLN1("~D3D7Object: d3dObject = 0x%x\n", d3dObject);
    if (d3dObject) {
	d3dObject->Release();
	d3dObject = NULL;
    }
}

HRESULT D3D7Object::CreateDevice(REFCLSID rclsid, DXSurface *dxSurface,
			     	 D3DDevice **d3dDevice)

{
    DTRACE_PRINTLN1("D3D7Object::CreateDevice: dxSurf = 0x%x",
    		    dxSurface);
    IDirect3DDevice7 *realD3dDevice;
    HRESULT d3dResult = d3dObject->CreateDevice(rclsid, 
	((DX7Surface*)dxSurface)->GetDDSurface(), &realD3dDevice);
    if (d3dResult == D3D_OK) {
	*d3dDevice = new D3D7Device(realD3dDevice);
    }
    return d3dResult;
}

/**
 * D3DDevice
 */
 
/**
 * Callback function used during texture enumeration.  Simply check the
 * incoming pixel format against our requirements and just keep returning
 * OK if the format is not suitable.  CANCEL means we have found our match
 * and we are done.
 */
HRESULT CALLBACK D3DDevice::TextureSearchCallback(DDPIXELFORMAT *lpddpf,
						  void *param)
{
    if(NULL == lpddpf || NULL == param )
        return DDENUMRET_OK;
    DDPIXELFORMAT pddpf = *lpddpf;
    TEXTURESEARCHINFO* ptsi = (TEXTURESEARCHINFO*)param;
    if (pddpf.dwFlags & DDPF_ALPHAPIXELS) {
	DTRACE_PRINTLN3("TextureSearch: bpp=%d alphadepth=%d alphamask=%x", 
                	pddpf.dwRGBBitCount, 
                	pddpf.dwAlphaBitDepth, 
                	pddpf.dwRGBAlphaBitMask);
    }
    if (ptsi->bUseAlpha && (pddpf.dwFlags & DDPF_ALPHAPIXELS) && 
        pddpf.dwRGBBitCount == ptsi->dwDesiredBPP)
    {
        WORD wBits = 0;
        DWORD dwMask=pddpf.dwRGBAlphaBitMask;
        while( dwMask ) {
            dwMask = dwMask & ( dwMask - 1 ); 
            wBits++;
        }
        if (wBits == ptsi->wDesiredAlphaBits) {

            memcpy(ptsi->pddpf, &pddpf, sizeof(DDPIXELFORMAT));
            ptsi->bFoundGoodFormat = TRUE;
            return DDENUMRET_CANCEL;
        }
    } else {

    }
    return DDENUMRET_OK;
}

/**
 * D3D5Device
 */
D3D5Device::D3D5Device(IDirect3DDevice2 *d3dDevice, 
		       IDirect3DViewport2 *d3dViewport)
{
    DTRACE_PRINTLN2("D3D5Device Constructor: d3dDevice, d3dVP = 0x%x, 0x%x",
    		    d3dDevice, d3dViewport);
    this->d3dDevice = d3dDevice;
    this->lpD3DViewport = d3dViewport;
    ZeroMemory(&viewData, sizeof(viewData));
    viewData.dwSize = sizeof(D3DVIEWPORT2);
    viewData.dwX = 0;
    viewData.dwY = 0;
    viewData.dvClipX = 0;
    viewData.dvClipY = 0;
    viewData.dvMinZ = 0.0f;
    viewData.dvMaxZ = 1.0f;
    viewportInitialized = FALSE;
    endScenePending = FALSE;
}

D3D5Device::~D3D5Device()
{
    if (lpD3DViewport) {
	lpD3DViewport->Release();
	lpD3DViewport = NULL;
    }
    if (d3dDevice) {
	d3dDevice->Release();
	d3dDevice = NULL;
    }
}

HRESULT D3D5Device::SetRenderTarget(DXSurface *dxSurface) {
    ForceEndScene();
    DX5Surface *dx5Surface = (DX5Surface*)dxSurface;
    return d3dDevice->SetRenderTarget(dx5Surface->GetDDSurface(), 0);
}

HRESULT D3D5Device::SetupViewport(int left, int top, int width, int height) {
    HRESULT ddResult = D3D_OK;
    if (!viewportInitialized) {
	ddResult = d3dDevice->AddViewport(lpD3DViewport);
	if (ddResult != D3D_OK) {
	    DebugPrintDirectDrawError(ddResult, "AddViewport failed");
	    return ddResult;
	}
	ddResult = d3dDevice->SetCurrentViewport(lpD3DViewport);
	if (ddResult != D3D_OK) {
	    DebugPrintDirectDrawError(ddResult, "SetCurrentViewport");
	    return ddResult;
	}
	viewportInitialized = TRUE;
    }
    viewData.dwX = left;
    viewData.dwY = top;
    viewData.dvClipX = (float)left;
    viewData.dvClipY = (float)top;
    viewData.dwWidth  = width;
    viewData.dwHeight = height;
    viewData.dvClipWidth = (float)viewData.dwWidth;
    viewData.dvClipHeight = (float)viewData.dwHeight;

    ddResult = lpD3DViewport->SetViewport2(&viewData);
    if (ddResult != D3D_OK) {
	DebugPrintDirectDrawError(ddResult, "SetViewport2 failed");
	return ddResult;
    }
    return ddResult;
}

HRESULT D3D5Device::SetupTextureRendering(D3DDeviceContext *d3dContext,
					  DXSurface *dxSurface) {

    HRESULT ddResult = D3D_OK;

    // REMIND: might want to cache stuff here, if it's the same texture surface
    D3DTEXTUREHANDLE d3dTexHandle;
    LPDIRECT3DTEXTURE2 lpD3DTexture2;
    LPDIRECTDRAWSURFACE lpSurface = ((DX5Surface*)dxSurface)->GetDDSurface();

    DX_FUNC(ddResult = lpSurface->QueryInterface(IID_IDirect3DTexture2,
						 (void**)&lpD3DTexture2));
    DX_FUNC(ddResult = lpD3DTexture2->GetHandle(d3dDevice, &d3dTexHandle));
    // don't need the interface once we have got a handle
    lpD3DTexture2->Release();

    if (ddResult != D3D_OK) {
	fprintf(stderr,"D3D5Device::SetupTextureRendering: Error getting Texture handle\n");
	return DDERR_GENERIC;
    }

    // Turn on alpha blending
    DX_FUNC(ddResult = d3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, d3dTexHandle));
    DX_FUNC(ddResult = d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE));

    DX_FUNC(ddResult = d3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA));
    // Translucency rule: dest = (src*1) + (dest*(1-srcAlpha);
    //  note that we use image type with premultiplied alpha for textures
    DX_FUNC(ddResult = d3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA));
    DX_FUNC(ddResult = d3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA));
    return ddResult;
}

HRESULT D3D5Device::UnsetTextureRendering(D3DDeviceContext *d3dContext) {
    HRESULT ddResult;

    // Turn off alpha blending
    DX_FUNC(ddResult = 
	    d3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL));
    DX_FUNC(ddResult = 
	    d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
    return ddResult;
}

HRESULT D3D5Device::BeginScene() 
{
    if (!d3dDevice) {
	return DDERR_GENERIC;
    } else {
	if (!endScenePending) {
	    return d3dDevice->BeginScene();
	}
	return D3D_OK;
    }
}

HRESULT D3D5Device::EndScene(HRESULT ddResult) {
    if (ddResult != D3D_OK) {
	endScenePending = FALSE;
	return d3dDevice->EndScene();
    }
    endScenePending = TRUE;
    return D3D_OK;
}

HRESULT D3D5Device::ForceEndScene() {
    if (endScenePending) {
	endScenePending = FALSE;
	return d3dDevice->EndScene();
    }
    return D3D_OK;
}

D3D7Device::D3D7Device(IDirect3DDevice7 *d3dDevice)
{
    DTRACE_PRINTLN2("D3D7Device Constructor: d3dDevice, this = 0x%x, 0x%x",
    		    d3dDevice, this);
    this->d3dDevice = d3dDevice;
    ZeroMemory(&viewData, sizeof(viewData));
    viewData.dwX = 0;
    viewData.dwY = 0;
    viewData.dvMinZ = 0.0f;
    viewData.dvMaxZ = 1.0f;
    d3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
    endScenePending = FALSE;
}

D3D7Device::~D3D7Device()
{
    DTRACE_PRINTLN1("~D3D7Device: d3dDevice = 0x%x", d3dDevice);
    if (d3dDevice) {
	d3dDevice->Release();
	d3dDevice = NULL;
    }
}

HRESULT D3D7Device::SetupTextureRendering(D3DDeviceContext *d3dContext,
					  DXSurface *dxSurface) {
    HRESULT ddResult = D3D_OK;
    // Turn on alpha blending
    DX_FUNC(ddResult = d3dDevice->SetTexture(0, ((DX7Surface*)dxSurface)->GetDDSurface()));
    if (ddResult != DD_OK) return ddResult;
    DX_FUNC(ddResult = d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,  TRUE));
    if (ddResult != DD_OK) return ddResult;
    DX_FUNC(ddResult = d3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA));
    if (ddResult != DD_OK) return ddResult;
    DX_FUNC(ddResult = d3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA));
    if (ddResult != DD_OK) return ddResult;
    DX_FUNC(ddResult = d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));

    return ddResult;
}

HRESULT D3D7Device::UnsetTextureRendering(D3DDeviceContext *d3dContext) {
    HRESULT ddResult;
    // Turn on off alpha blending and remove the texture to prevent
    // memory leak
    DX_FUNC(ddResult = d3dDevice->SetTexture(0, NULL));
    if (ddResult != DD_OK) return ddResult;
    DX_FUNC(ddResult = 
	    d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,  FALSE));
    return ddResult;
}

HRESULT D3D7Device::SetRenderTarget(DXSurface *dxSurface) {
    DTRACE_PRINTLN2("D3D7Device::SetRenderTarget: d3d, dxSurf = 0x%x, 0x%x",
    		    d3dDevice, dxSurface);
    ForceEndScene();
    DX7Surface *dx7Surface = (DX7Surface*)dxSurface;
    return d3dDevice->SetRenderTarget(dx7Surface->GetDDSurface(), 0);
}

HRESULT D3D7Device::SetupViewport(int left, int top, int width, int height) 
{
    DTRACE_PRINTLN3("D3D7Device::SetupViewport: d3d, w, h = 0x%x, %d, %d",
    		    d3dDevice, width, height);
    HRESULT ddResult = D3D_OK;
    viewData.dwX = left;
    viewData.dwY = top;
    viewData.dwWidth  = width;
    viewData.dwHeight = height;    
    ddResult = d3dDevice->SetViewport(&viewData);
    if (ddResult != D3D_OK) {
	DebugPrintDirectDrawError(ddResult, "D3D7Device::SetupViewport failed");
	return ddResult;
    }
    return ddResult;
}

HRESULT D3D7Device::BeginScene() 
{
    if (!d3dDevice) {
	return DDERR_GENERIC;
    } else {
	if (!endScenePending) {
	    return d3dDevice->BeginScene();
	}
	return D3D_OK;
    }
}

HRESULT D3D7Device::EndScene(HRESULT ddResult) {
    if (ddResult != D3D_OK) {
	endScenePending = FALSE;
	return d3dDevice->EndScene();
    }
    endScenePending = TRUE;
    return D3D_OK;
}

HRESULT D3D7Device::ForceEndScene() {
    if (endScenePending) {
	endScenePending = FALSE;
	return d3dDevice->EndScene();
    }
    return D3D_OK;
}



/**
 * D3DDeviceContext
 */
D3DDeviceContext::D3DDeviceContext(DXSurface *dxSurface,
				   D3DObject *d3dObject)
{
    GetExclusiveAccess();
    vpWidth = 0;
    vpHeight = 0;
    d3dDevice = NULL;
    this->d3dObject = d3dObject;
    SetNew3DDevice(dxSurface);
    DTRACE_PRINTLN1("D3DDeviceContext const: d3dDevice = 0x%x",
    		    d3dDevice);
    ReleaseExclusiveAccess();
}

void D3DDeviceContext::Release3DDevice() {
    GetExclusiveAccess();
    DTRACE_PRINTLN1("D3DDeviceContext::Release3DDevice: d3dDevice = 0x%x",
    		    d3dDevice);
    if (d3dDevice) {
        delete d3dDevice;
        d3dDevice = NULL;
    }
    ReleaseExclusiveAccess();
}    

D3DDeviceContext::~D3DDeviceContext() {
    DTRACE_PRINTLN2("~D3DDeviceContext: d3dDevice, d3dObject = 0x%x, 0x%x\n",
    		    d3dDevice, d3dObject);
    Release3DDevice();
    GetExclusiveAccess();
    delete d3dObject;
    d3dObject = NULL;
    ReleaseExclusiveAccess();
}

HRESULT D3DDeviceContext::SetNew3DDevice(DXSurface *dxSurface) {
    GetExclusiveAccess();

    Release3DDevice();
    lpTargetSurface = NULL;
    found16bppFormat = FALSE;
    found32bppFormat = FALSE;

    HRESULT ddResult = d3dObject->CreateDevice(IID_IDirect3DHALDevice, 
                                               dxSurface,
                                               &d3dDevice);
    if (ddResult != D3D_OK) {
        d3dDevice = NULL;
        DebugPrintDirectDrawError(ddResult, "Can't create d3d device");
    } else {
        TEXTURESEARCHINFO tsi;
        ZeroMemory(&tsi, sizeof(TEXTURESEARCHINFO));
        tsi.pddpf            = &textureFormat16bpp;
        tsi.bUseAlpha        = TRUE;
        tsi.wDesiredAlphaBits = 4;
        tsi.dwDesiredBPP     = 16;
        tsi.bFoundGoodFormat = FALSE;
        d3dDevice->EnumTextureFormats(&tsi);
        if (tsi.bFoundGoodFormat) {
            found16bppFormat = TRUE;
        } else {
            DTRACE_PRINTLN("Can't find 16-bit pixel format");
        }

        ZeroMemory(&tsi, sizeof(TEXTURESEARCHINFO));
        tsi.pddpf            = &textureFormat32bpp;
        tsi.bUseAlpha        = TRUE;
        tsi.wDesiredAlphaBits = 8;
        tsi.dwDesiredBPP     = 32;
        tsi.bFoundGoodFormat = FALSE;
        d3dDevice->EnumTextureFormats(&tsi);
        if (tsi.bFoundGoodFormat) {
            found32bppFormat = TRUE;
        } else {
            DTRACE_PRINTLN("Can't find 32-bit pixel format");
        }
    }
    ReleaseExclusiveAccess();
    return ddResult;
}

/**
 * This method is supposed to be called inside Get/ReleaseExclusiveAccess(),
 * so we're not locking inside.
 */
HRESULT D3DDeviceContext::SetRenderTarget(DXSurface *dxSurface,
                                          int left, int top, 
                                          int width, int height) 
{
    HRESULT ddResult = D3D_OK;
    if (d3dDevice == NULL) {
        return DDERR_GENERIC;
    }

    if (lpTargetSurface != dxSurface) {
	if ((ddResult = d3dDevice->SetRenderTarget(dxSurface)) != D3D_OK) {
	    DebugPrintDirectDrawError(ddResult, "SetRenderTarget failed");
	    return ddResult;
	}
	lpTargetSurface = dxSurface;
    }

    ddResult = SetupViewport(left, top, width, height);
    return ddResult;
}

HRESULT D3DDeviceContext::SetupViewport(int left, int top, 
					int width, int height) {
    DTRACE_PRINTLN4("SetupViewport: oldLeft, oldTop, oldW, oldH = %d, %d, %d, %d",
    		    vpLeft, vpTop, vpWidth, vpHeight);
    DTRACE_PRINTLN4("               left, top, W, H = %d, %d, %d, %d",
    		    left, top, width, height);
    HRESULT ddResult = D3D_OK;
    if (d3dDevice == NULL) {
        return DDERR_GENERIC;
    }
    if (vpLeft != left || vpTop != top ||
    	vpWidth != width || vpHeight != height) 
    {
        ddResult = d3dDevice->SetupViewport(left, top, width, height);
        if (ddResult != D3D_OK) {
            DebugPrintDirectDrawError(ddResult, "SetViewport2 failed");
            return ddResult;
        }
        vpLeft = left;
        vpTop = top;
        vpWidth = width;
        vpHeight = height;
    }
    return ddResult;
}

LPDDPIXELFORMAT D3DDeviceContext::GetPixelFormat(int depth) {
    LPDDPIXELFORMAT format = NULL;
    if (depth == 16 && found16bppFormat) {
        format = &textureFormat16bpp;
    } else if (depth == 32 && found32bppFormat) {
        format = &textureFormat32bpp;
    } else {
        DTRACE_PRINTLN1("GetPixelFormat: wrong texture depth"
			" or no pixel format found for depth %d\n", depth);
    }
    return format;
}

D3DDevice *D3DDeviceContext::Get3DDevice() {
    return d3dDevice;
}
