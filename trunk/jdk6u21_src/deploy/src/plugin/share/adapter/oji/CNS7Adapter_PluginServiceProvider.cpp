/*
 * @(#)CNS7Adapter_PluginServiceProvider.cpp	1.4 03/05/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS7Adapter_PluginServiceProvider.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS7Adapter_PluginServiceProvider.cpp is Implementation of providing 
// various service (plugin/liveconnect/jvm) from browser to adapter.
// 
//
#include "StdAfx.h"
#include "jni.h"
#include "IJVMManager.h"
#include "IPluginManager.h"
#include "IThreadManager.h"
#include "ILiveconnect.h"
#include "IBrowserAuthenticator.h"
#include "nsIServiceManager.h"
#include "nsIPluginManager.h"
#include "nsIJVMManager.h"
#include "nsILiveconnect.h"
#include "nsIObserverService.h"
#include "nsIJVMAuthTools.h"
#include "nsIComponentManager.h"
#include "CNSAdapter_JVMManager.h"
#include "CNSAdapter_PluginManager.h"
#include "CNSAdapter_Liveconnect.h"
#include "CNSAdapter_BrowserAuthenticator.h"
#include "CNS7Adapter_ObserverService.h"
#include "CNS7Adapter_PluginServiceProvider.h"

#include "Debug.h"

// Since Mozilla does not define this class id anywhere in the interfaces exposed
// we need to clone this in order to be able to use the service 
#define NS_OBSERVERSERVICE_CID \
    { 0xd07f5195, 0xe3d1, 0x11d2, { 0x8a, 0xcd, 0x0, 0x10, 0x5a, 0x1b, 0x88, 0x60 } }

#define NS_COMPONENTMANAGER_CID                      \
{ /* 91775d60-d5dc-11d2-92fb-00e09805570f */		\
    0x91775d60,                                      \
    0xd5dc,                                          \
    0x11d2,                                          \
    {0x92, 0xfb, 0x00, 0xe0, 0x98, 0x05, 0x57, 0x0f} \
}

// Class id/Interface id of JPI interfaces 
static JD_DEFINE_IID(jIJVMManagerIID, IJVMMANAGER_IID);
static JD_DEFINE_IID(jIPluginManagerIID, IPLUGINMANAGER_IID);
static JD_DEFINE_IID(jIThreadManagerIID, ITHREADMANAGER_IID);
static JD_DEFINE_IID(jILiveconnectIID, ILIVECONNECT_IID);
static JD_DEFINE_IID(jISupportsIID, ISUPPORTS_IID);
static JD_DEFINE_CID(jCJVMManagerCID, CJVMMANAGER_CID);
static JD_DEFINE_CID(jCPluginManagerCID, CPLUGINMANAGER_CID);
static JD_DEFINE_CID(jCLiveconnectCID, CLIVECONNECT_CID);
static JD_DEFINE_CID(jCObserverServiceCID, COBSERVERSERVICE_CID);

// Class id/Interface id of Mozilla interfaces
static NS_DEFINE_IID(kIServiceManagerIID, NS_ISERVICEMANAGER_IID);
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kIPluginManagerIID, NS_IPLUGINMANAGER_IID);
static NS_DEFINE_IID(kIJVMManagerIID, NS_IJVMMANAGER_IID);
static NS_DEFINE_IID(kIComponentManagerIID, NS_ICOMPONENTMANAGER_IID);
static NS_DEFINE_IID(kILiveConnectIID, NS_ILIVECONNECT_IID);
static JD_DEFINE_CID(jIBrowserAuthenticatorIID, IBROWSERAUTHENTICATOR_IID);
static JD_DEFINE_CID(jIBrowserAuthenticatorCID, IBROWSERAUTHENTICATOR_IID);


static NS_DEFINE_CID(kCPluginManagerCID, NS_PLUGINMANAGER_CID);
static NS_DEFINE_CID(kCJVMManagerCID, NS_JVMMANAGER_CID);
static NS_DEFINE_CID(kCLiveConnectCID, NS_CLIVECONNECT_CID);
static NS_DEFINE_IID(kComponentManagerCID, NS_COMPONENTMANAGER_CID);
static NS_DEFINE_IID(kIObserverServiceIID, NS_IOBSERVERSERVICE_IID);
static NS_DEFINE_CID(kCObserverServiceCID, NS_OBSERVERSERVICE_CID);
static NS_DEFINE_CID(kIJVMAuthToolsCID, NS_JVMAUTHTOOLS_CID);
static NS_DEFINE_IID(kIJVMAuthToolsIID, NS_IJVMAUTHTOOLS_IID);

//------------------------------------------------------------------------------------
// ISupports Methods
//------------------------------------------------------------------------------------

JD_IMPL_ISUPPORTS1(CNS7Adapter_PluginServiceProvider, IPluginServiceProvider);

//------------------------------------------------------------------------------------
// CNS7Adapter_PluginServiceProvider::CNS7Adapter_PluginServiceProvider
//------------------------------------------------------------------------------------
CNS7Adapter_PluginServiceProvider::CNS7Adapter_PluginServiceProvider(nsISupports* pProvider)
    : mService(NULL), 
      m_pPluginManager(NULL), m_pJVMManager(NULL), m_pComponentManager(NULL),
      m_pPluginManagerAdapter(NULL), m_pJVMManagerAdapter(NULL)

{
    TRACE("CNS7Adapter_PluginServiceProvider::CPluginServiceProviderAdpter");

    JD_INIT_REFCNT();

    // Try to obtain nsIServiceManager
    // We can obtain m_pPluginManager & m_pJVMManager now, however,
    // this will delay the start-up time, so it is better not to do this.
    pProvider->QueryInterface(kIServiceManagerIID, (void**)&mService);
}


//------------------------------------------------------------------------------------
// CNS7Adapter_PluginServiceProvider::~CNS7Adapter_PluginServiceProvider
//------------------------------------------------------------------------------------
CNS7Adapter_PluginServiceProvider::~CNS7Adapter_PluginServiceProvider(void)
{
    TRACE("CNS7Adapter_PluginServiceProvider::~CNS7Adapter_PluginServiceProvider");
    if (mService != NULL)
    {
	if (m_pPluginManager != NULL)
	{
	    // Netscape 7's nsIServiceManager has got rid of ReleaseService method
	    //mService->ReleaseService(kCPluginManagerCID, m_pPluginManager, NULL);
	    m_pPluginManager->Release();
	}

	if (m_pJVMManager != NULL)
	{
	    //mService->ReleaseService(kCJVMManagerCID, m_pJVMManager, NULL);
	    m_pJVMManager->Release();
	}

	if (m_pComponentManager != NULL)
	{
	    //mService->ReleaseService(kComponentManagerCID, m_pComponentManager, NULL);
	    // Mozilla does not addRef componentManager
	    //m_pComponentManager->Release();
	}

	if (m_pPluginManagerAdapter != NULL)
	    m_pPluginManagerAdapter->Release();

	if (m_pJVMManagerAdapter != NULL)
	    m_pJVMManagerAdapter->Release();

	mService->Release();
    }
}


//------------------------------------------------------------------------------------
// CNS7Adapter_PluginServiceProvider::QueryService
//------------------------------------------------------------------------------------
//
// Obtain a plugin service. An nsIPluginServiceProvider will obtain the 
// plugin service according to the current browser version.
//
// @param clsid - the CLSID of the requested service.
// @param iid - the IID of the request service.
// @param result - the interface pointer of the requested service
// @return - JD_OK if this operation was successful.
//
// Notes:
//
JD_METHOD
CNS7Adapter_PluginServiceProvider::QueryService(/*[in]*/ const JDCID& clsid,
					        /*[in]*/ const JDIID& iid,
				               /*[out]*/ ISupports* *result) 
{
    TRACE("CNS7Adapter_PluginServiceProvider::QueryService");

    if (result == NULL || mService == NULL)
	return JD_ERROR_NULL_POINTER;

    *result = NULL;

    JDresult res;
	
    // Try it from cache
    if(clsid.Equals(jCPluginManagerCID) && m_pPluginManagerAdapter != NULL)
	return m_pPluginManagerAdapter->QueryInterface(iid, (void**)result);
    
    else if (clsid.Equals(jCJVMManagerCID) && m_pJVMManagerAdapter != NULL)
	return m_pJVMManagerAdapter->QueryInterface(iid, (void**)result);

    else if (clsid.Equals(jCLiveconnectCID) && m_pComponentManager != NULL)
    {
	// Use componentManager to create liveconnect for each applet instance.
	JDSmartPtr<nsILiveconnect> spLiveconnectInst;
	res = m_pComponentManager->CreateInstance(kCLiveConnectCID, NULL, kILiveConnectIID, (void**)&spLiveconnectInst);
		    
	if (NS_FAILED(res))
	    return res;
	
	// Wrap nsILiveconnect into the adapter object
	JDSmartPtr<ILiveconnect> spLiveconnectAdptInst = new CNSAdapter_Liveconnect(spLiveconnectInst);
	
	if (spLiveconnectAdptInst == NULL)
	    return JD_ERROR_OUT_OF_MEMORY;
	    
	return spLiveconnectAdptInst->QueryInterface(iid, (void**)result);
    }
    
    else if (clsid.Equals(jCObserverServiceCID))
    {
	JDSmartPtr<nsIObserverService> spObserverService;
	res = mService->GetService(kCObserverServiceCID, kIObserverServiceIID, (void**)&spObserverService);
	if (NS_SUCCEEDED(res) && spObserverService)
	    *result = new CNS7Adapter_ObserverService(spObserverService);
	
	if (*result == NULL)
	    return JD_ERROR_OUT_OF_MEMORY;

	(*result)->AddRef();

	return res;
    } 
	else if(clsid.Equals(jIBrowserAuthenticatorCID)) {
		JDSmartPtr<nsIJVMAuthTools> spIJVMAuthTools;
		res = mService->GetService(kIJVMAuthToolsCID, kIJVMAuthToolsIID, (void**)&spIJVMAuthTools);
		if(NS_FAILED(res))
				return res;
		JDSmartPtr<IBrowserAuthenticator> spBrowserAuthenticator = 
			new CNSAdapter_BrowserAuthenticator((nsIJVMAuthTools*)spIJVMAuthTools);
		if(spBrowserAuthenticator == NULL)
				return JD_ERROR_OUT_OF_MEMORY;

		return spBrowserAuthenticator->QueryInterface(iid, (void**)result);
	}
    
    // It is not good idea to use smart pointer since GetService won't AddRef the
    // resultant pointer.
    nsISupports* pMozService;

    nsCID requestCID = kCPluginManagerCID;
    // We need to do some conversion here between clsid from JPI and clsid of Mozilla
    if (clsid.Equals(jCPluginManagerCID))
	requestCID = kCPluginManagerCID;
    else if (clsid.Equals(jCJVMManagerCID))
	requestCID = kCJVMManagerCID;
    else if (clsid.Equals(jCLiveconnectCID))
	requestCID = kComponentManagerCID;
    else
	return JD_ERROR_FAILURE;

    res = mService->GetService(requestCID, kISupportsIID, (void**)&pMozService);
   

    // Try to cache the service object
    if (NS_SUCCEEDED(res) && pMozService != NULL)
    {			
	if (clsid.Equals(jCPluginManagerCID) && m_pPluginManager == NULL)
	{
	    res = pMozService->QueryInterface(kIPluginManagerIID, (void**)&m_pPluginManager);
	    if (NS_FAILED(res))
		return res;

	    m_pPluginManagerAdapter = new CNSAdapter_PluginManager(m_pPluginManager);
	    
	    if (m_pPluginManagerAdapter == NULL)
		return JD_ERROR_OUT_OF_MEMORY;

	    m_pPluginManagerAdapter->AddRef();
	    return m_pPluginManagerAdapter->QueryInterface(iid, (void**)result);
	}
	else if (clsid.Equals(jCJVMManagerCID) && m_pJVMManager == NULL)
	{
	    res = pMozService->QueryInterface(kIJVMManagerIID, (void**)&m_pJVMManager);
	    if (NS_FAILED(res))
		return res;

	    m_pJVMManagerAdapter = new CNSAdapter_JVMManager(m_pJVMManager);

	    if (m_pJVMManagerAdapter == NULL)
		return JD_ERROR_OUT_OF_MEMORY;

	    m_pJVMManagerAdapter->AddRef();
	    return m_pJVMManagerAdapter->QueryInterface(iid, (void**)result);	
	}
	else if (clsid.Equals(jCLiveconnectCID) && m_pComponentManager == NULL){
	    // Since mComponentManager is here only to create the liveconnect instance
	    // so iid must be jILiveconnectIID
 	    if (!iid.Equals(jILiveconnectIID))
		return JD_ERROR_FAILURE; 
	    
	    res = pMozService->QueryInterface(kIComponentManagerIID, (void**)&m_pComponentManager);
	    if (NS_FAILED(res))
		return res;

	    JDSmartPtr<nsILiveconnect> spLiveconnectInst;
	    res = m_pComponentManager->CreateInstance(kCLiveConnectCID, NULL, kILiveConnectIID, (void**)&spLiveconnectInst);

	    if (NS_SUCCEEDED(res) && spLiveconnectInst != NULL)
	    {
		JDSmartPtr<ILiveconnect> spLiveconnectAdptInst = new CNSAdapter_Liveconnect(spLiveconnectInst);
		if (spLiveconnectAdptInst == NULL)
		    return JD_ERROR_OUT_OF_MEMORY;
	    
		return spLiveconnectAdptInst->QueryInterface(iid, (void**)result);
	    }
	    
	}
    }
    return res;
}


//------------------------------------------------------------------------------------
// CNS7Adapter_PluginServiceProvider::ReleaseService
//------------------------------------------------------------------------------------
// Release a plugin service. An nsIPluginServiceProvider will release the 
// plugin service according to the current browser version.
//
// @param clsid - the CLSID of the service.
// @param result - the interface pointer of the service
// @return - JD_OK if this operation was successful.
//
// Notes:
//
JD_METHOD
CNS7Adapter_PluginServiceProvider::ReleaseService(/*[in]*/ const JDCID& clsid, 
					          /*[in]*/ ISupports* pService)
{
    TRACE("CNS7Adapter_PluginServiceProvider::ReleaseService\n");

    if (pService == NULL)
	return JD_ERROR_NULL_POINTER;

    if (mService != NULL)
    {
	// We are running inside Mozilla and have nsIServiceManager
	if (clsid.Equals(jCPluginManagerCID) && m_pPluginManager != NULL)  {
	    return pService->Release();
	}
	else if (clsid.Equals(jCJVMManagerCID) && m_pJVMManager != NULL)  {
	    return pService->Release();
	}
	else if (clsid.Equals(jCLiveconnectCID) && m_pComponentManager != NULL)  {
	    return pService->Release();
	}
	else  {
	    // Unknown service
	    return JD_ERROR_FAILURE;
	}
    }
    return JD_ERROR_FAILURE;
}
