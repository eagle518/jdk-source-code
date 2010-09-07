/*
 * @(#)CNS7Adapter.cpp	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS7Adapter.cpp  by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS7Adapter.cpp : Implementation of the CNS7Adapter that is the OJI 
//		     Plug-in COM object.
//

#include "StdAfx.h"
#include "jni.h"
#include "IPluginModule.h"
#include "nsIFactory.h"
#include "IFactory.h"
#include "CNS7Adapter_PluginServiceProvider.h"
#include "CNS7Adapter_JavaPluginFactory.h"
#include "clientLoadCOM.h"
#include "oji_clsid.h"
#include "CNS7Adapter.h"
#include "Debug.h"

const LPCSTR NSGetFactory_NAME = "NSGetFactory";

typedef JDresult (*NSGetFactoryProc)(IPluginServiceProvider* pPluginServiceProvider,
				     IFactory** factory);

////////////////////////////////////////////////////////////////////////////////////
// CNS7Adapter
STDMETHODIMP_(nsresult) CNS7Adapter::NSGetFactory(const nsCID &aClass,
						  nsISupports *pProvider,
						  nsIFactory **factory)
{
    TRACE("CNS7Adapter::NSGetFactory\n");

    if (factory == NULL)
	return NS_ERROR_NULL_POINTER;

    static NSGetFactoryProc pProc = NULL;

    if (pProc == NULL)
    {
	HINSTANCE hDLL = NULL;
   
	// Load JPINS32.DLL
	if (LoadPluginLibrary(JPINS_SERVER_NAME, &hDLL) != ERROR_SUCCESS)
	    return NS_ERROR_FAILURE;

	// Get the function address of "NSGetFactory" exposed by JPINS32.DLL
	pProc = (NSGetFactoryProc)
		GetProcAddress( hDLL, NSGetFactory_NAME );

        if (pProc == NULL)
            return NS_ERROR_FAILURE;
    }

    JDSmartPtr<IFactory> spIFactory;
    JDSmartPtr<IPluginServiceProvider> spPluginServiceProvider(new CNS7Adapter_PluginServiceProvider(pProvider));
    
    if (spPluginServiceProvider == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    JDresult res = pProc(spPluginServiceProvider, &spIFactory);

    if (JD_SUCCEEDED(res) && spIFactory != NULL)
    {
	// Construct an adapter for JavaPluginFactory object
	*factory = new CNS7Adapter_JavaPluginFactory(spIFactory);

	if (*factory == NULL)
	    return NS_ERROR_OUT_OF_MEMORY;
    
	(*factory)->AddRef();
    }
    return NS_OK;
}

STDMETHODIMP_(PRBool) CNS7Adapter::NSCanUnload(void)
{
    TRACE("CNSAdapter::NSCanUnload\n");

    return NS_OK;
}


STDMETHODIMP_(nsresult) CNS7Adapter::NSRegisterSelf(const char* path)
{
    TRACE("CNSAdapter::NSRegisterSelf\n");

    return NS_OK;
}


STDMETHODIMP_(nsresult) CNS7Adapter::NSUnregisterSelf(const char* path)
{
    TRACE("CNSAdapter::NSUnregisterSelf\n");

    return NS_OK;
}
  

  
