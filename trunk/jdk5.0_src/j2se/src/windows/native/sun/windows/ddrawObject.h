/*
 * @(#)ddrawObject.h	1.31 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef DDRAWOBJECT_H
#define DDRAWOBJECT_H

#include <ddraw.h>
#include <d3d.h>
#include <jni.h>
#include <windows.h>
#include "Win32SurfaceData.h"
#include "d3dObject.h"
#include "GraphicsPrimitiveMgr.h"

#ifdef DEBUG
  #define DX_FUNC(func) do { \
  	  HRESULT ddr = (func); \
  	  if (ddr != DD_OK) \
  	      DebugPrintDirectDrawError(ddr, #func); \
  } while (0)
#else
  #define DX_FUNC(func) do { func; } while (0)
#endif

/**
 * Class for display modes
 */
class DDrawDisplayMode {

public:
    typedef void (*Callback)(DDrawDisplayMode&, void*);

public:
    DDrawDisplayMode();
    DDrawDisplayMode(DDrawDisplayMode& rhs);
    DDrawDisplayMode(jint w, jint h, jint b, jint r);
    virtual ~DDrawDisplayMode();

    jint width;
    jint height;
    jint bitDepth;
    jint refreshRate;

};

class DDrawSurface;
class DDrawClipper;
class DXSurface;

class DX5Surface;
class DX7Surface;
class DDrawSurfaceDesc;
class D3DDeviceContext;
class D3DDevice;
class D3D5Device;
class D3D7Device;
class D3DObject;
class D3D5Object;
class D3D7Object;


class DXObject {
public:
    DXObject() {};
    virtual ~DXObject() {}
    virtual HRESULT GetCaps(LPDDCAPS halCaps, LPDDCAPS helCaps) = 0;
    virtual HRESULT GetAvailableVidMem(DWORD caps, DWORD *total, 
				       DWORD *free) = 0;
    virtual HRESULT CreateSurface(DWORD dwFlags, DWORD ddsCaps,
				  LPDDPIXELFORMAT lpPf,
    				  int width, int height,
    				  DXSurface **lpDDSurface,
    				  int numBackBuffers) = 0;
    virtual HRESULT CreateSurface(DWORD dwFlags, DWORD ddsCaps,
				  LPDDPIXELFORMAT lpPf,
    				  int width, int height,
    				  DXSurface **lpDDSurface) 
    {
	return CreateSurface(dwFlags, ddsCaps, lpPf, width, height, lpDDSurface, 0);
    }
    virtual HRESULT CreateSurface(DWORD dwFlags, DWORD ddsCaps,
    				  DXSurface **lpDDSurface) 
    {
	return CreateSurface(dwFlags, ddsCaps, NULL, 0, 0, lpDDSurface, 0);
    }
    virtual HRESULT CreateSurface(DWORD dwFlags, DWORD ddsCaps,
    				  DXSurface **lpDDSurface,
    				  int numBackBuffers) 
    {
	return CreateSurface(dwFlags, ddsCaps, NULL, 0, 0, lpDDSurface, 
			     numBackBuffers);
    }
    virtual HRESULT CreateClipper(DWORD dwFlags, 
				  LPDIRECTDRAWCLIPPER FAR *lplpDDClipper) = 0;
    virtual HRESULT GetDisplayMode(DDrawDisplayMode &dm) = 0;
    virtual HRESULT SetDisplayMode(DWORD width, DWORD height, DWORD depth, 
				   DWORD refreshRate) = 0;
    virtual HRESULT EnumDisplayModes(DDrawDisplayMode *dm, 
				     DDrawDisplayMode::Callback callback,
				     void *context) = 0;
    virtual HRESULT RestoreDisplayMode() = 0;
    virtual HRESULT SetCooperativeLevel(HWND hWnd, DWORD dwFlags) = 0;
    virtual D3DObject *CreateD3DObject() = 0;
    /**
     * Structure for enumerating display modes; used to invoke the callback
     */
    class EnumDisplayModesParam {
    public:
        EnumDisplayModesParam(DDrawDisplayMode::Callback cb, void* ct);
        virtual ~EnumDisplayModesParam();
        DDrawDisplayMode::Callback callback;
        void* context;
    };

};

class DX7Object : public DXObject {
private:
    IDirectDraw7 *ddObject;

    static long WINAPI EnumCallback(LPDDSURFACEDESC2 pDDSD, void* pContext);

public:
    DX7Object(IDirectDraw7 *ddObject);
    virtual ~DX7Object();
    virtual HRESULT GetCaps(LPDDCAPS halCaps, LPDDCAPS helCaps) {
	return ddObject->GetCaps(halCaps, helCaps);
    }
    virtual HRESULT GetAvailableVidMem(DWORD caps, DWORD *total, 
				       DWORD *free);
    virtual HRESULT CreateSurface(DWORD dwCaps, DWORD ddsCaps,
				  LPDDPIXELFORMAT lpPf,
    				  int width, int height,
    				  DXSurface **lpDDSurface,
    				  int numBackBuffers);
    virtual HRESULT CreateClipper(DWORD dwFlags, 
				  LPDIRECTDRAWCLIPPER FAR *lplpDDClipper)
    {
	return ddObject->CreateClipper(dwFlags, lplpDDClipper, NULL);
    }
    virtual HRESULT GetDisplayMode(DDrawDisplayMode &dm);	
    virtual HRESULT SetDisplayMode(DWORD width, DWORD height, DWORD depth, 
				   DWORD refreshRate)
    {
	return ddObject->SetDisplayMode(width, height, depth, refreshRate, 0);
    }
    virtual HRESULT EnumDisplayModes(DDrawDisplayMode *dm, 
    				     DDrawDisplayMode::Callback callback,
    				     void *context);
    virtual HRESULT RestoreDisplayMode() {
	return ddObject->RestoreDisplayMode();
    }
    virtual HRESULT SetCooperativeLevel(HWND hWnd, DWORD dwFlags) {
	return ddObject->SetCooperativeLevel(hWnd, (dwFlags | 
						    DDSCL_FPUPRESERVE));
    }
    virtual D3DObject *CreateD3DObject();
};

class DX5Object : public DXObject {
private:
    IDirectDraw2 *ddObject;
    
    static long WINAPI EnumCallback(LPDDSURFACEDESC pDDSD, void* pContext);

public:
    DX5Object(IDirectDraw2 *ddObject);
    virtual ~DX5Object();
    virtual HRESULT GetCaps(LPDDCAPS halCaps, LPDDCAPS helCaps) {
	return ddObject->GetCaps(halCaps, helCaps);
    }
    virtual HRESULT GetAvailableVidMem(DWORD caps, DWORD *total, 
				       DWORD *free);
    virtual HRESULT CreateSurface(DWORD dwCaps, DWORD ddsCaps,
				  LPDDPIXELFORMAT lpPf,
    				  int width, int height,
    				  DXSurface **lpDDSurface,
    				  int numBackBuffers);
    virtual HRESULT CreateClipper(DWORD dwFlags, 
				  LPDIRECTDRAWCLIPPER FAR *lplpDDClipper)
    {
	return ddObject->CreateClipper(dwFlags, lplpDDClipper, NULL);
    }
    virtual HRESULT GetDisplayMode(DDrawDisplayMode &dm);	
    virtual HRESULT SetDisplayMode(DWORD width, DWORD height, DWORD depth, 
				   DWORD refreshRate)
    {
	return ddObject->SetDisplayMode(width, height, depth, refreshRate, 0);
    }
    virtual HRESULT EnumDisplayModes(DDrawDisplayMode *dm, 
    				     DDrawDisplayMode::Callback callback,
    				     void *context);
    virtual HRESULT RestoreDisplayMode() {
	return ddObject->RestoreDisplayMode();
    }
    virtual HRESULT SetCooperativeLevel(HWND hWnd, DWORD dwFlags) {
	return ddObject->SetCooperativeLevel(hWnd, dwFlags);
    }
    virtual D3DObject *CreateD3DObject();
};


typedef HRESULT (WINAPI *FnDDCreateFunc)(GUID FAR * lpGUID,
    LPDIRECTDRAW FAR * lplpDD, IUnknown FAR * pUnkOuter);
typedef HRESULT (WINAPI *FnDDCreateExFunc)(GUID FAR * lpGUID,
    LPVOID * lplpDD, REFIID refIID, IUnknown FAR * pUnkOuter);
/**
 * Class for the direct draw object
 */
class DDraw  {

private:

public:
    DDraw(DXObject *dxObject);
    virtual ~DDraw();
    
    static DDraw *CreateDDrawObject(GUID *lpGUID);

    BOOL GetDDCaps(LPDDCAPS caps);
    HRESULT GetDDAvailableVidMem(DWORD *free);
    DDrawSurface* CreateDDOffScreenSurface(DWORD width, DWORD height,
					   DWORD depth,
					   jboolean isVolatile,
                                           jint transparency,
					   DWORD surfaceTypeCaps);
    DDrawSurface* CreateDDPrimarySurface(DWORD backBufferCount);
    D3DDeviceContext *CreateD3dContext(DDrawSurface *ddSurface);
    void DisableD3D() { deviceUseD3D = FALSE; }
    BOOL IsD3DEnabled() { return deviceUseD3D; }
    DDrawClipper* CreateDDClipper();

    BOOL GetDDDisplayMode(DDrawDisplayMode& dm);
    HRESULT SetDDDisplayMode(DDrawDisplayMode& dm);
    BOOL EnumDDDisplayModes(DDrawDisplayMode* constraint,
        DDrawDisplayMode::Callback callback, void* context);
    BOOL RestoreDDDisplayMode();

    HRESULT SetCooperativeLevel(HWND hwnd, DWORD dwFlags);

private:
    DXObject		    *dxObject;
    DDrawSurface	    *lpPrimary;
    D3DDeviceContext        *d3dContext;
    BOOL                    deviceUseD3D;
};


#define VERSION_DX5	0x00000005
#define VERSION_DX7	0x00000007

/**
 * Abstract base class for all ddraw surface operations
 * Actual calls into ddraw will be made in the subclasses of
 * this class, using the actual ddraw surface objects.  This 
 * holder class allows DDrawSurface to perform all operations
 * on surfaces generically without having to worry about what
 * version of the DIRECTDRAWSURFACE interface we are actually
 * instantiated with.
 */
class DXSurface {
    
    // Implementation detail: All calls methods of this class are
    // as close as possible to actual DIRECTDRAWSURFACE methods, but
    // remove all references to interfaces in the argument list.
    // So, for example, the call to Blt uses a DDrawSurfaceHolder
    // source instead of a DIRECTDRAWSURFACE source; this allows
    // the instance of this class to unwrap that holder appropriately
    // to use the correct ddraw interface.
public:
    DXSurface() { clipperSet = FALSE; }
    
    virtual HRESULT Blt(RECT *destRect, DXSurface *lpSurfaceSrc, 
			RECT *srcRect, DWORD dwFlags, LPDDBLTFX ddBltFx) = 0;
    virtual HRESULT Lock(RECT *lockRect, SurfaceDataRasInfo *pRasInfo, 
    			 DWORD dwFlags, HANDLE hEvent) = 0;
    virtual HRESULT Unlock(RECT *unlockRect) = 0;
    virtual HRESULT Flip(DWORD dwFlags) = 0;
    virtual HRESULT IsLost() = 0;
    virtual HRESULT Restore() = 0;
    virtual HRESULT GetDC(HDC *hDC) = 0;
    virtual HRESULT ReleaseDC(HDC hDC) = 0;
    virtual ULONG   Release() = 0;
    virtual HRESULT SetClipper(DDrawClipper *pClipper) = 0;
    virtual HRESULT SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) = 0;
    virtual HRESULT GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) = 0;
    virtual HRESULT GetAttachedSurface(DWORD dwCaps, DXSurface **bbSurface) = 0;
	    DWORD   GetVersionID() { return versionID; }
    virtual int     GetSurfaceDepth() = 0;

protected:
    DWORD versionID;
    BOOL clipperSet;
};

/**
 * DXSurface implementation for DX 5 interfaces
 * (DIRECTDRAWSURFACE and DIRECT3DDEVICE2)
 */
class DX5Surface : public DXSurface {
public:
    IDirectDrawSurface *lpSurface;
    DDSURFACEDESC ddsd;

public:
    DX5Surface() { versionID = VERSION_DX5; }
    
    DX5Surface(IDirectDrawSurface *lpSurface);

    IDirectDrawSurface *GetDDSurface() { return lpSurface; }

    virtual HRESULT Blt(RECT *destRect, DXSurface *lpSurfaceSrc, 
			RECT *srcRect, DWORD dwFlags, LPDDBLTFX ddBltFx)
    {
	return lpSurface-> Blt(destRect, 
			       lpSurfaceSrc ?
			           ((DX5Surface*)lpSurfaceSrc)->GetDDSurface() :
			           NULL,
			       srcRect, dwFlags, ddBltFx);
    }
    virtual HRESULT Lock(RECT *lockRect, SurfaceDataRasInfo *pRasInfo, DWORD dwFlags, 
			 HANDLE hEvent);
    virtual HRESULT Unlock(RECT *unlockRect) {
	return lpSurface->Unlock(unlockRect);
    }

    virtual HRESULT Flip(DWORD dwFlags) {
	return lpSurface->Flip(NULL, dwFlags);
    }
    virtual HRESULT IsLost() {
	return lpSurface->IsLost();
    }
    virtual HRESULT Restore() {
	return lpSurface->Restore();
    }
    virtual HRESULT GetDC(HDC *hDC) {
	return lpSurface->GetDC(hDC);
    }
    virtual HRESULT ReleaseDC(HDC hDC) {
	return lpSurface->ReleaseDC(hDC);
    }
    virtual ULONG Release() {
	return lpSurface->Release();
    }
    virtual HRESULT SetClipper(DDrawClipper *pClipper);
    virtual HRESULT SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) {
	return lpSurface->SetColorKey(dwFlags, lpDDColorKey);
    }
    virtual HRESULT GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) {
	return lpSurface->GetColorKey(dwFlags, lpDDColorKey);
    }
    virtual HRESULT GetAttachedSurface(DWORD dwCaps, DXSurface **bbSurface);
    virtual int     GetSurfaceDepth();
};

/**
 * DXSurface implementation for DX 7 interfaces
 * (IDirectDrawSurface7)
 */
class DX7Surface : public DXSurface {
public:
    IDirectDrawSurface7 *lpSurface;
    DDSURFACEDESC2 ddsd;

public:
    DX7Surface() { versionID = VERSION_DX7; }
    
    DX7Surface(IDirectDrawSurface7 *lpSurface);

    IDirectDrawSurface7 *GetDDSurface() { return lpSurface; }
    virtual HRESULT Blt(RECT *destRect, DXSurface *lpSurfaceSrc, 
			RECT *srcRect, DWORD dwFlags, LPDDBLTFX ddBltFx)
    {
	return lpSurface->Blt(destRect, 
			      lpSurfaceSrc ?
			          ((DX7Surface*)lpSurfaceSrc)->GetDDSurface() :
			          NULL,
			      srcRect, dwFlags, ddBltFx);
    }
    virtual HRESULT Lock(RECT *lockRect, SurfaceDataRasInfo *pRasInfo, 
    			 DWORD dwFlags, HANDLE hEvent);
    virtual HRESULT Unlock(RECT *unlockRect) {
	return lpSurface->Unlock(unlockRect);
    }
    virtual HRESULT Flip(DWORD dwFlags) {
	return lpSurface->Flip(NULL, dwFlags);
    }
    virtual HRESULT IsLost() {
	return lpSurface->IsLost();
    }
    virtual HRESULT Restore() {
	return lpSurface->Restore();
    }
    virtual HRESULT GetDC(HDC *hDC) {
	return lpSurface->GetDC(hDC);
    }
    virtual HRESULT ReleaseDC(HDC hDC) {
	return lpSurface->ReleaseDC(hDC);
    }
    virtual ULONG Release() {
	return lpSurface->Release();
    }
    virtual HRESULT SetClipper(DDrawClipper *pClipper);
    virtual HRESULT SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) {
	return lpSurface->SetColorKey(dwFlags, lpDDColorKey);
    }
    virtual HRESULT GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) {
	return lpSurface->GetColorKey(dwFlags, lpDDColorKey);
    }
    virtual HRESULT GetAttachedSurface(DWORD dwCaps, DXSurface **bbSurface);
    virtual int     GetSurfaceDepth();
};


/**
 * Class for direct draw surfaces
 */
class DDrawSurface {

    friend class DDraw;
    friend class DDrawPrimarySurface;
    friend class DDrawBackBufferSurface;

protected:
    DDrawSurface(DXSurface *dxSurface, D3DDeviceContext *d3dContext, 
    		 int width, int height);
    DDrawSurface();

public:
    virtual ~DDrawSurface();

public:
    virtual void    SetNewSurface(DXSurface *dxSurface, int width, int height);
    virtual HRESULT ReleaseSurface();
    virtual HRESULT SetClipper(DDrawClipper* pClipper);
    virtual HRESULT SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey);
    virtual HRESULT GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey);
    virtual HRESULT Lock(LPRECT lockRect = NULL, SurfaceDataRasInfo *pRasInfo = NULL,
        DWORD dwFlags = DDLOCK_WAIT, HANDLE hEvent = NULL);
    virtual HRESULT Unlock(LPRECT lockRect = NULL);
    virtual HRESULT Blt(LPRECT destRect, DDrawSurface* pSrc,
        LPRECT srcRect = NULL, DWORD dwFlags = DDBLT_WAIT,
        LPDDBLTFX lpDDBltFx = NULL);
    virtual HRESULT DrawLineStrip(D3DTLVERTEX *lpVerts, int numVerts, 
				  BOOL drawLastPixel, int clipX1, int clipY1,
				  int clipX2, int clipY2);
    virtual HRESULT TextureBlt(RECT *rSrc, RECT *rDst, DDrawSurface* pSrc,
			       CompositeInfo *compInfo);
    virtual HRESULT Flip(DDrawSurface* pDest, DWORD dwFlags = DDFLIP_WAIT);
    virtual HRESULT IsLost();
    virtual HRESULT Restore();
    virtual HDC GetDC();
    virtual HRESULT ReleaseDC(HDC hDC);
    void    GetExclusiveAccess() { CRITICAL_SECTION_ENTER(*surfaceLock); };
    void    ReleaseExclusiveAccess() { CRITICAL_SECTION_LEAVE(*surfaceLock); };
    virtual int GetWidth() { return width; }
    virtual int GetHeight() { return height; }
    virtual DDrawSurface* GetDDAttachedSurface(DWORD caps = 0)
	{ return NULL; };
    virtual DXSurface *GetDXSurface() { return dxSurface; }
    void    SetD3dContext(D3DDeviceContext *ctxt);
    void    FlushD3DContext();
    int     GetSurfaceDepth();

protected:
    DXSurface	*dxSurface;
    CriticalSection *surfaceLock;
    D3DDeviceContext *d3dContext;
    int 	width, height;
    // REMIND : handle SURFACEDESC
};

class BackBufferHolder;

/**
 * Class for direct draw primary surface
 */
class DDrawPrimarySurface : DDrawSurface {

    friend class DDraw;

protected:
    BackBufferHolder *bbHolder;
    DXSurface *backBuffer;
    
protected:
    DDrawPrimarySurface(DXSurface *dxSurface, 
    			D3DDeviceContext *d3dContext,
    			int width, int height);
    DDrawPrimarySurface();

public:
    virtual ~DDrawPrimarySurface();
    virtual HRESULT ReleaseSurface();
    virtual void    SetNewSurface(DXSurface *dxSurface, int width, int height);
    virtual DDrawSurface* GetDDAttachedSurface(DWORD caps = 0);
    virtual void ReleaseBackBuffer();
    virtual HRESULT Restore();
};

/**
 * Class for direct draw back buffer surface
 */
class DDrawBackBufferSurface : DDrawSurface {

    friend class DDraw;
    friend class DDrawPrimarySurface;

protected:
    DDrawPrimarySurface *lpPrimary;
    BackBufferHolder *bbHolder;
    
protected:
    DDrawBackBufferSurface(BackBufferHolder *holder, 
    			   D3DDeviceContext *d3dContext,
    			   int width, int height);
    DDrawBackBufferSurface();

public:
    virtual ~DDrawBackBufferSurface();
    virtual HRESULT ReleaseSurface();
};

/**
 * Linked list holding all references to DDrawBackBufferSurface
 * objects that share a single ddraw surface.  This class
 * is used by BackBufferHolder.
 */
class BackBufferList {
public:
    DDrawBackBufferSurface *backBuffer;
    BackBufferList *next;
};

/** 
 * Class for storing the shared ddraw/d3d back buffer objects
 * and a list of all objects that use those shared surfaces.
 */
class BackBufferHolder {
    
public:
    BackBufferHolder(DXSurface *dxSurface);
    BackBufferHolder();    
    ~BackBufferHolder();	
    
    virtual void Add(DDrawBackBufferSurface *surf);
    virtual void Remove(DDrawBackBufferSurface *surf);
    DXSurface *GetBackBufferSurface() { return backBuffer; };
    
protected:
    BackBufferList *bbList;		// linked list of objects that
    					// share the ddraw/d3d surfaces
    DXSurface *backBuffer;
    CriticalSection bbLock;		// synchronize accesses to list
};
    

struct TEXTURESEARCHINFO
{
	DWORD dwDesiredBPP; // Input for texture format search
	WORD  wDesiredAlphaBits;
	BOOL  bUseAlpha;
	BOOL  bFoundGoodFormat;
	DDPIXELFORMAT* pddpf; // Result of texture format search
};



#ifdef DEBUG
void StackTrace();
// Critical Section debugging class
class DDCriticalSection : public CriticalSection {
private:
    DDrawSurface* lpSurface;
    int count;

public:
    DDCriticalSection(DDrawSurface* surface) : lpSurface(surface), count(0) {
    }
    void Enter() {
        ++count;
        //DTRACE_PRINTLN2("Enter DDCriticalSection for surface 0x%x count %d\n",
        //    lpSurface, count);
        CriticalSection::Enter();
    }
    void Leave() {
        //DTRACE_PRINTLN2("Leave DDCriticalSection for surface 0x%x count %d\n",
        //    lpSurface, count);
        if (count == 0) {
            //DTRACE_PRINTLN1(
            //    "Invalid decrement in DDCriticalSection for surface 0x%x\n",
            //    lpSurface);
            StackTrace();
        }
        CriticalSection::Leave();
        count--;
    }
};
#else
#define DDCriticalSection(x) CriticalSection()
#define StackTrace()
#endif

/**
 * Class for direct draw clippers
 */
class DDrawClipper {

    friend class DDraw;

private:
    DDrawClipper(LPDIRECTDRAWCLIPPER clipper);

public:
    virtual ~DDrawClipper();

public:
    HRESULT SetHWnd(DWORD dwFlags, HWND hwnd);
    HRESULT GetClipList(LPRECT lpRect, LPRGNDATA rgnData, LPDWORD rgnSize);
    LPDIRECTDRAWCLIPPER GetClipper();

private:
    LPDIRECTDRAWCLIPPER lpClipper;
};


#endif DDRAWOBJECT_H
