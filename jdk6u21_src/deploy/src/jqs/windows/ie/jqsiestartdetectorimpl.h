/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#pragma once
#include "resource.h"       // main symbols

#include "JQSIEStartDetector.h"
#include <shlguid.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CJQSIEStartDetectorImpl

class ATL_NO_VTABLE CJQSIEStartDetectorImpl :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CJQSIEStartDetectorImpl, &CLSID_JQSIEStartDetectorImpl>,
    public IObjectWithSiteImpl<CJQSIEStartDetectorImpl>,
    public IDispatchImpl<IJQSIEStartDetectorImpl, &IID_IJQSIEStartDetectorImpl, &LIBID_JQSIEStartDetectorLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
    CJQSIEStartDetectorImpl()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_JQSIESTARTDETECTORIMPL)

DECLARE_NOT_AGGREGATABLE(CJQSIEStartDetectorImpl)

BEGIN_COM_MAP(CJQSIEStartDetectorImpl)
    COM_INTERFACE_ENTRY(IJQSIEStartDetectorImpl)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()



    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

public:
    STDMETHOD(SetSite)(IUnknown *pUnkSite);

};

OBJECT_ENTRY_AUTO(__uuidof(JQSIEStartDetectorImpl), CJQSIEStartDetectorImpl)
