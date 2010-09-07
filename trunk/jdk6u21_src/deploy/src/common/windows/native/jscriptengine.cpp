/*
 * @(#)jscriptengine.cpp	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// jscriptengine.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//

#include "stdafx.h"
#include <activscp.h>
#include <comdef.h>	   // COM smart pointer
#include "jscriptengine.h"
#include "jscriptsite.h"


/**
 * Instantiate the JScript engine. 
 */
HRESULT JScriptEngine::Initialize(void)
{
    HRESULT hr;

    if (m_spScript)
	return E_FAIL;   // Already created it

    CLSID clsidEngine;

    // JScript is the ProgID of the ActiveX Scripting engine in IE4
    hr = ::CLSIDFromProgID(L"JScript", &clsidEngine);
    if (FAILED(hr))
    {
	// JavaScript is the ProgID of the ActiveX Scripting engine in IE3
	hr = ::CLSIDFromProgID(L"JavaScript", &clsidEngine);

	if (FAILED(hr))
	    return hr;
    }

    // Create the ActiveX Scripting Engine
    hr = CoCreateInstance(clsidEngine, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void **)&m_spScript);
    if (hr)
    {
#ifdef _DEBUG
	OutputDebugString(TEXT("Failed to create JScript engine."));
#endif
	return E_FAIL;
    }

    return hr;
}

/**
 * Evaluate JavaScript expression.
 *
 * @parem [in] lpwszScript JavaScript expression
 * @parem [out] lpwszResult evaluation result
 * @return S_OK if successful.
 */
HRESULT JScriptEngine::Eval(LPCWSTR lpwszScript, LPWSTR lpwszResult)
{
    HRESULT hr = E_FAIL;

    if (m_spScript == NULL)
      return E_FAIL;

    IActiveScriptPtr spScript;

    // Clone the scripting engine.
    hr = m_spScript->Clone(&spScript);
    if (FAILED(hr))
    {
#ifdef _DEBUG
	OutputDebugString(TEXT("Unable to clone JScript engine."));
#endif
	return hr;	
    }

    
    IActiveScriptParsePtr spScriptParse;
    
    // Script Engine must support IActiveScriptParse for us to use it
    hr = spScript->QueryInterface(IID_IActiveScriptParse, (void **)&spScriptParse);
    if (FAILED(hr))
    {
#ifdef _DEBUG
	OutputDebugString(TEXT("JScript engine doesn't support IActiveScriptParse."));
#endif
	return hr;
    }
  
    IActiveScriptSitePtr spScriptSite;

    CComObject<JScriptSite>* pJScriptSite = NULL;
    CComObject<JScriptSite>::CreateInstance(&pJScriptSite);

    pJScriptSite->QueryInterface(IID_IActiveScriptSite, (void**)&spScriptSite); 

    // Set scripting site
    hr = spScript->SetScriptSite(spScriptSite);
    if (hr)
	return hr;

    // InitNew the object:
    hr = spScriptParse->InitNew();

    VARIANT v;
    VariantInit(&v);

    // Feed the custom JavaScript to the JavaScript engine.
    hr = spScriptParse->ParseScriptText(lpwszScript, NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISEXPRESSION , &v, NULL);

    if (v.vt == VT_BSTR)
    {
	wsprintfW(lpwszResult, L"%s", v.bstrVal);
    }
    
    VariantClear(&v);

    return hr;
}


