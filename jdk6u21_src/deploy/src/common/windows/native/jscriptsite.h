/*
 * @(#)jscriptsite.h	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// jscriptsite.h    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//

#ifndef __JSCRIPTSITE_H__
#define __JSCRIPTSITE_H__

/////////////////////////////////////////////////////////////////////////////
// JScriptSite is an event sink for the JScript engine.
//
class ATL_NO_VTABLE JScriptSite : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IActiveScriptSite
{
public:
    JScriptSite()
    {
    }

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(JScriptSite)
	COM_INTERFACE_ENTRY(IActiveScriptSite)
END_COM_MAP()

// IActiveScriptSite
public:

    STDMETHODIMP GetLCID(/* [out] */ LCID *plcid)
    {
	return E_NOTIMPL;	// Use system settings
    }
        
    STDMETHODIMP GetItemInfo(/* [in] */ LPCOLESTR pstrName,
		    	     /* [in] */ DWORD dwReturnMask,
			     /* [out] */ IUnknown **ppiunkItem,
			     /* [out] */ ITypeInfo **ppti)
    {
	return TYPE_E_ELEMENTNOTFOUND;  // No custom scripting object is supported.
    }
        
    STDMETHODIMP GetDocVersionString(/* [out] */ BSTR  *pbstrVersion)
    {
	return E_NOTIMPL;	
    }
        
    STDMETHODIMP OnScriptTerminate(/* [in] */ const VARIANT *pvarResult,
			           /* [in] */ const EXCEPINFO *pexcepinfo)
    {
	return S_OK;
    }
        
    STDMETHODIMP OnStateChange(/* [in] */ SCRIPTSTATE ssScriptState)
    {
	return S_OK;
    }
        
    STDMETHODIMP OnScriptError(/* [in] */ IActiveScriptError *pScriptError)
    {
	USES_CONVERSION;

	EXCEPINFO excepInfo;

	if (pScriptError)
	{
	    // Obtain error information
	    HRESULT hr = pScriptError->GetExceptionInfo(&excepInfo);

	    if (SUCCEEDED(hr))
	    {
		LPSTR lpszSource = NULL, lpszDescription = NULL;
		
		lpszSource = W2A(excepInfo.bstrSource);

		if (excepInfo.bstrDescription)
		    lpszDescription = W2A(excepInfo.bstrDescription);
		
		MessageBox(NULL, lpszDescription, lpszSource, MB_OK | MB_ICONEXCLAMATION);
		
		::SysFreeString(excepInfo.bstrSource);
		::SysFreeString(excepInfo.bstrDescription);
		::SysFreeString(excepInfo.bstrHelpFile);
	    }
	}

	return S_OK;
    }
        
    STDMETHODIMP OnEnterScript(void)
    {
	return S_OK;
    }
        
    STDMETHODIMP OnLeaveScript(void)
    {
	return S_OK;
    }
};


#endif  // __JSCRIPTSITE_H__
