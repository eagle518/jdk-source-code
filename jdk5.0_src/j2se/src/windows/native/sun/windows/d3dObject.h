/*
 * @(#)d3dObject.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef D3DOBJECT_H
#define D3DOBJECT_H

#include "ddrawObject.h"

class DXSurface;
class D3DDevice;
class DX5Surface;
class DX7Surface;
class D3DDeviceContext;

class D3DObject {
public:
    virtual ~D3DObject() {}
    virtual HRESULT CreateDevice(REFCLSID rclsid, DXSurface *dxSurface,
    				 D3DDevice **d3dDevice) = 0;
};

class D3D5Object : public D3DObject {
    IDirect3D2 *d3dObject;
public:
    virtual ~D3D5Object();
    D3D5Object(IDirect3D2 *d3dObject) { this->d3dObject = d3dObject; }
    
    virtual HRESULT CreateDevice(REFCLSID rclsid, DXSurface *dxSurface,
    				 D3DDevice **d3dDevice);
};

class D3D7Object : public D3DObject {
    IDirect3D7 *d3dObject;
public:
    virtual ~D3D7Object();
    D3D7Object(IDirect3D7 *d3dObject) { this->d3dObject = d3dObject; }
    
    virtual HRESULT CreateDevice(REFCLSID rclsid, DXSurface *dxSurface,
    				 D3DDevice **d3dDevice);
};

class D3DDevice {
public:
    virtual ~D3DDevice() {}
    virtual HRESULT Release() = 0;
    static  HRESULT CALLBACK TextureSearchCallback(DDPIXELFORMAT *lpddpf, void *param);
    virtual HRESULT EnumTextureFormats(void *tsi) = 0;
    virtual HRESULT SetupTextureRendering(D3DDeviceContext *d3dContext,
					  DXSurface *dxSurface) = 0;
    virtual HRESULT UnsetTextureRendering(D3DDeviceContext *d3dContext) = 0;
    virtual HRESULT SetRenderTarget(DXSurface *dxSurface) = 0;
    virtual HRESULT SetupViewport(int left, int top, int width, int height) = 0;
    virtual HRESULT DrawPrimitive(D3DPRIMITIVETYPE primType, 
				  void *verts, DWORD numVerts) = 0;
    virtual HRESULT BeginScene() = 0;
    virtual HRESULT EndScene(HRESULT ddResult) = 0;
    virtual HRESULT SetRenderState(D3DRENDERSTATETYPE dwRenderType,
				   DWORD dwRenderState) { return D3D_OK;};

    virtual HRESULT ForceEndScene() = 0;
    BOOL    endScenePending;
};

class D3D5Device : public D3DDevice {
    IDirect3DDevice2 *d3dDevice;
    D3DVIEWPORT2 viewData;
    IDirect3DViewport2 *lpD3DViewport;
    BOOL viewportInitialized;
    
public:
    virtual ~D3D5Device();
    D3D5Device(IDirect3DDevice2 *d3dDevice, IDirect3DViewport2 *d3dViewport);
    
    virtual HRESULT Release() { return d3dDevice->Release(); }
    static  HRESULT CALLBACK TextureSearchCallback(DDSURFACEDESC *ddsd, 
    						   void *param)
    {
	if (!ddsd || !param) {
	    return DDENUMRET_OK;
	} else {
	    return D3DDevice::TextureSearchCallback(&(ddsd->ddpfPixelFormat), 
	    					    param);
	}
    }
    virtual HRESULT EnumTextureFormats(void *tsi) {
	return d3dDevice->EnumTextureFormats(D3D5Device::TextureSearchCallback,
					     tsi);
    }
    virtual HRESULT SetupTextureRendering(D3DDeviceContext *d3dContext,
					  DXSurface *dxSurface);
    virtual HRESULT UnsetTextureRendering(D3DDeviceContext *d3dContext);
    virtual HRESULT SetRenderTarget(DXSurface *dxSurface);
    virtual HRESULT SetupViewport(int left, int top, int width, int height);
    virtual HRESULT DrawPrimitive(D3DPRIMITIVETYPE primType, 
				  void *verts, DWORD numVerts)
    {
	return d3dDevice->DrawPrimitive(primType, (D3DVERTEXTYPE)D3DVT_TLVERTEX,
					verts,
					numVerts, D3DDP_DONOTCLIP);
    }
    virtual HRESULT BeginScene();
    virtual HRESULT EndScene(HRESULT ddResult);
    virtual HRESULT ForceEndScene();
};
class D3D7Device : public D3DDevice {
    IDirect3DDevice7 *d3dDevice;
    D3DVIEWPORT7 viewData;
public:
    virtual ~D3D7Device();
    D3D7Device(IDirect3DDevice7 *d3dDevice);
    virtual HRESULT Release() { return d3dDevice->Release(); }
    virtual HRESULT EnumTextureFormats(void *tsi) {
	return d3dDevice->EnumTextureFormats(D3DDevice::TextureSearchCallback,
					     tsi);
    }
    virtual HRESULT SetupTextureRendering(D3DDeviceContext *d3dContext,
					  DXSurface *dxSurface);
    virtual HRESULT UnsetTextureRendering(D3DDeviceContext *d3dContext);
    virtual HRESULT SetRenderTarget(DXSurface *dxSurface);
    virtual HRESULT SetupViewport(int left, int top, int width, int height);
    virtual HRESULT DrawPrimitive(D3DPRIMITIVETYPE primType, 
				  void *verts, DWORD numVerts)
    {
	return d3dDevice->DrawPrimitive(primType, D3DFVF_TLVERTEX, verts, 
					numVerts, 0);
    }
    virtual HRESULT BeginScene();
    virtual HRESULT EndScene(HRESULT ddResult);
    virtual HRESULT ForceEndScene();
};

class D3DDeviceContext {
public:
    D3DDeviceContext(DXSurface *dxSurface, D3DObject *d3dObject);
    virtual ~D3DDeviceContext();
    void GetExclusiveAccess() { deviceLock.Enter(); }
    void ReleaseExclusiveAccess() { deviceLock.Leave(); }
    
    D3DDevice *Get3DDevice();
    LPDDPIXELFORMAT GetPixelFormat(int depth);
    HRESULT SetRenderTarget(DXSurface *dxSurface,
                            int left, int top, int width, int height);
    HRESULT SetNew3DDevice(DXSurface *dxSurface);
    void Release3DDevice();
    
protected:
    HRESULT SetupViewport(int left, int top, int width, int height);

private:
    DXSurface *lpTargetSurface;
    D3DDevice *d3dDevice;
    D3DObject *d3dObject;

    DDPIXELFORMAT           textureFormat16bpp;
    DDPIXELFORMAT           textureFormat32bpp;
    BOOL                    found16bppFormat;
    BOOL                    found32bppFormat;

    int vpLeft;
    int vpTop;
    int vpWidth;
    int vpHeight;

    CriticalSection deviceLock;
};


#endif D3DOBJECT_H
