/*
 * @(#)CNS6Adapter.cpp	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS6Adapter.cpp  by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS6Adapter.cpp : Implementation of the CNS6Adapter that is the OJI 
//		       Plug-in COM object.
//

#include "stdafx.h"
#include "jni.h"
#include "nsIFactory.h"
#include "IFactory.h"
#include "CNS6Adapter_PluginServiceProvider.h"
#include "CNS6Adapter_JavaPluginFactory.h"
#include "CNS6Adapter.h"
#include "Debug.h"
#include "clientLoadCOM.h"
#include "oji_clsid.h"

const LPCSTR NSGetFactory_NAME = "NSGetFactory";

typedef JDresult (*NSGetFactoryProc)(IPluginServiceProvider* pPluginServiceProvider,
				     IFactory** factory);


/////////////////////////////////////////////////////////////////////////////
// CNS6Adapter

STDMETHODIMP_(nsresult) CNS6Adapter::NSGetFactory(const nsCID &aClass,
						  nsISupports *pProvider,
						  nsIFactory **factory)
{
    TRACE("CNS6Adapter::NSGetFactory\n");

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
    JDSmartPtr<IPluginServiceProvider> spPluginServiceProvider(new CNS6Adapter_PluginServiceProvider(pProvider));
    
    if (spPluginServiceProvider == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    JDresult res = pProc(spPluginServiceProvider, &spIFactory);

    if (JD_SUCCEEDED(res) && spIFactory != NULL)
    {
	// Construct an adapter for JavaPluginFactory object
	*factory = new CNS6Adapter_JavaPluginFactory(spIFactory);

	if (*factory == NULL)
	    return NS_ERROR_OUT_OF_MEMORY;
    
	(*factory)->AddRef();
    }
    return NS_OK;
}

STDMETHODIMP_(PRBool) CNS6Adapter::NSCanUnload(void)
{
    TRACE("CNS6Adapter::NSCanUnload\n");

    return NS_OK;
}


STDMETHODIMP_(nsresult) CNS6Adapter::NSRegisterSelf(const char* path)
{
    TRACE("CNS6Adapter::NSRegisterSelf\n");

    return NS_OK;
}


STDMETHODIMP_(nsresult) CNS6Adapter::NSUnregisterSelf(const char* path)
{
    TRACE("CNS6Adapter::NSUnregisterSelf\n");

    return NS_OK;
}

