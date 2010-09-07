/*
 * @(#)isInstalled.h	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// isInstalled.h : Declaration of the CisInstalled

#ifndef __ISINSTALLED_H_
#define __ISINSTALLED_H_

#include "resource.h"       // main symbols
#include <atlctl.h>

#define JAVAWS_KEY "JNLPFile\\Shell\\Open\\Command"
#define JAVAWS_CFG_ARG "-c"

typedef int (PASCAL FAR *WSACleanupType)(VOID);
extern WSACleanupType do_WSACleanup;

/////////////////////////////////////////////////////////////////////////////
// CisInstalled
class ATL_NO_VTABLE CisInstalled : 
	public CComObjectRootEx<CComSingleThreadModel>,	
	public IDispatchImpl<IisInstalled, &IID_IisInstalled, &LIBID_JAVAWEBSTARTLib>,
	public CComControl<CisInstalled>,
	public IPersistStreamInitImpl<CisInstalled>,
	public IPersistPropertyBagImpl<CisInstalled>,
	public IOleControlImpl<CisInstalled>,
	public IOleObjectImpl<CisInstalled>,
	public IOleInPlaceActiveObjectImpl<CisInstalled>,
	public IViewObjectExImpl<CisInstalled>,
	public IOleInPlaceObjectWindowlessImpl<CisInstalled>,
	public IPersistStorageImpl<CisInstalled>,
	public ISpecifyPropertyPagesImpl<CisInstalled>,
	public IQuickActivateImpl<CisInstalled>,
	public IDataObjectImpl<CisInstalled>,
	public IProvideClassInfo2Impl<&CLSID_isInstalled, NULL, &LIBID_JAVAWEBSTARTLib>,
	public CComCoClass<CisInstalled, &CLSID_isInstalled>,
	public IObjectSafetyImpl<CisInstalled, INTERFACESAFE_FOR_UNTRUSTED_CALLER|INTERFACESAFE_FOR_UNTRUSTED_DATA> // renes: Marked as safe for scripting and initialization
{
public:
	CisInstalled()
	{
		// Don't allow windowless creation. Need this to ensure we get Windows
		// messges, especially WM_CREATE for initialization purposes.
		m_bWindowOnly = TRUE;
		m_closed = FALSE;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ISINSTALLED)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CisInstalled)
	COM_INTERFACE_ENTRY(IisInstalled)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY(IPersistPropertyBag)
	COM_INTERFACE_ENTRY_IID(IID_IPersist, IPersistPropertyBag)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY(IObjectSafety) // renes: Tie IObjectSafety to this COM Object
END_COM_MAP()

BEGIN_PROP_MAP(CisInstalled)
	// Do not use PROP_ENTRY macro for security reason
    // We now use IPropertyBag to get parameters
END_PROP_MAP()

BEGIN_MSG_MAP(CisInstalled)
	CHAIN_MSG_MAP(CComControl<CisInstalled>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()

// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IPersistPropertyBag (For reading parameters)
	STDMETHOD(IPersistPropertyBag_Load)(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog, ATL_PROPMAP_ENTRY* pMap);
	
// IisInstalled
public:
	STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions);
	STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);
	STDMETHOD(dnsResolve)(/*[in]*/ BSTR hostname, /*[out, retval]*/ BSTR *ipAddr);
	STDMETHOD(Close)(DWORD dwSaveOption);
	STDMETHOD(InPlaceActivate)(LONG iVerb, const RECT* prcPosRect);

	HRESULT OnDraw(ATL_DRAWINFO& di);

	CComBSTR m_app, m_dbg, m_back;
	BOOL m_closed;
};

#endif //__ISINSTALLED_H_
