/*
 * @(#)CNSAdapter_PluginManager.cpp	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_PluginManager.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_PluginManager.cpp is Implementation of adapter for nsIPluginManager
// 
//
#include "StdAfx.h"
#include "jni.h"
#include "nsIPluginManager.h"
#include "nsIPluginManager2.h"
#include "nsICookieStorage.h" 
#include "IPluginManager.h"
#include "IPluginInstance.h"
#include "IPluginStreamListener.h"
#include "ICookieStorage.h"

#include "CNSAdapter_PluginManager.h"
#include "CNSAdapter_JavaPlugin.h"
#include "CMap.h"
#include "Debug.h"

// Netscape iid
static NS_DEFINE_IID(kIPluginManagerIID, NS_IPLUGINMANAGER_IID);
static NS_DEFINE_IID(kIPluginManager2IID, NS_IPLUGINMANAGER2_IID);
static NS_DEFINE_IID(kICookieStorageIID, NS_ICOOKIESTORAGE_IID);

static JD_DEFINE_IID(jIPluginInstanceIID, IPLUGININSTANCE_IID);

// This map is to get the corresponding CNSAdapter_JavaPlugin object associated with CJavaPlugin object
extern CMap<void*, void*> pluginMap;

// ISupports
JD_IMPL_ISUPPORTS2(CNSAdapter_PluginManager, IPluginManager, ICookieStorage);

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginManager::CNSAdapter_PluginManager
//=--------------------------------------------------------------------------=
// params:  
//
// notes:
//
CNSAdapter_PluginManager::CNSAdapter_PluginManager(nsIPluginManager* pPluginManager) : 
    m_pPluginManager(pPluginManager) 
{
    TRACE("CNSAdapter_PluginManager::CNSAdapter_PluginManager\n");
    JD_INIT_REFCNT();

    if (m_pPluginManager)
	m_pPluginManager->AddRef();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginManager::~CNSAdapter_PluginManager
//=--------------------------------------------------------------------------=
// params:  
//
// notes:
//
CNSAdapter_PluginManager::~CNSAdapter_PluginManager() 
{
    TRACE("CNSAdapter_PluginManager::~CNSAdapter_PluginManager\n");
    if (m_pPluginManager)
	m_pPluginManager->Release();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginManager::GetValue
//=--------------------------------------------------------------------------=
// params: variable     
// value:     The value to get for the variable
//
// notes:
//
JD_METHOD 
CNSAdapter_PluginManager::GetValue(JDPluginManagerVariable variable, void* value)
{
    TRACE("CNSAdapter_PluginManager::GetValue\n");
    
    nsPluginManagerVariable pVariable;

    switch(variable) {
    case JDPluginManagerVariable_XDisplay:
        pVariable = nsPluginManagerVariable_XDisplay;
        break;
    case JDPluginManagerVariable_XtAppContext:
        pVariable = nsPluginManagerVariable_XtAppContext;
        break;
#ifdef NO_XEMBED_MOZILLA
    case JDPluginManagerVariable_SupportsXEmbed:
        pVariable = nsPluginManagerVariable_SupportsXEmbed;
        break;
#endif	// NO_XEMBED_MOZILLA
    default:
        return JD_ERROR_FAILURE;
    }

    return m_pPluginManager->GetValue(pVariable, value);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginManager::UserAgent
//=--------------------------------------------------------------------------=
// params:  resultingAgentString   the returned user-agent string
//
// notes:
//
JD_METHOD 
CNSAdapter_PluginManager::UserAgent(const char* *resultingAgentString)
{
    TRACE("CNSAdapter_PluginManager::UserAgent\n");

    if (m_pPluginManager == NULL)
	return JD_ERROR_NULL_POINTER;
	
    return m_pPluginManager->UserAgent(resultingAgentString);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginManager::GetURL
//=--------------------------------------------------------------------------=
// params:  
//
// notes:
//
JD_METHOD
CNSAdapter_PluginManager::GetURL(ISupports* pluginInst,
			         const char* url,
				 const char* target,
				 IPluginStreamListener* sl,
			         const char* altHost,
			         const char* referrer,
			         JDBool forceJSEnabled)
{
    TRACE("CNSAdapter_PluginManager::GetURL\n");

    if (m_pPluginManager == NULL || pluginInst == NULL)
	return JD_ERROR_NULL_POINTER;

    // the streamListener is not null only for Netscape 4.x browser
    if (sl != NULL)
	return JD_ERROR_FAILURE;

    JDSmartPtr<IPluginInstance> inst;
    
    if (JD_FAILED(pluginInst->QueryInterface(jIPluginInstanceIID, (void**)&inst)) )
        return JD_ERROR_FAILURE;

    CNSAdapter_JavaPlugin* pluginAdapterInst = (CNSAdapter_JavaPlugin*)pluginMap.FindElement(inst);
        
    if (pluginAdapterInst == NULL)
	return JD_ERROR_FAILURE;

    return m_pPluginManager->GetURL((nsIJVMPluginInstance*)pluginAdapterInst, url, target, NULL, altHost, referrer, forceJSEnabled);
}


//=--------------------------------------------------------------------------=
// CNSAdapter_PluginManager::FindProxyForURL
//=--------------------------------------------------------------------------=
// params:  url       the url 
//	    result    proxy used for the above url
//
// notes:
//
JD_METHOD
CNSAdapter_PluginManager::FindProxyForURL(const char* url, char* *result)
{
    TRACE("CNSAdapter_PluginManager::FindProxyForURL\n");

    if (m_pPluginManager == NULL)
	return JD_ERROR_NULL_POINTER;

    // "FindProxyForURL" is implemented in nsIPluginManager2 
    JDSmartPtr<nsIPluginManager2> spPluginManager2;
    nsresult res = m_pPluginManager->QueryInterface(kIPluginManager2IID, (void**)&spPluginManager2);

    if (NS_SUCCEEDED(res) && spPluginManager2)
	return spPluginManager2->FindProxyForURL(url, result);

    return res;
}


//=--------------------------------------------------------------------------=
// CNSAdapter_PluginManager::GetCookie
//=--------------------------------------------------------------------------=
// params:  
//
// notes:
//
JD_METHOD
CNSAdapter_PluginManager::GetCookie(const char* inCookieURL, void* inOutCookieBuffer, JDUint32& inOutCookieSize)
{
    TRACE("CNSAdapter_PluginManager::GetCookie\n");
    if (m_pPluginManager == NULL)
	return JD_ERROR_NULL_POINTER;
    
    // Query nsICookieStorage interface to make this call.
    JDSmartPtr<nsICookieStorage> spCookieStorage;
    nsresult res = m_pPluginManager->QueryInterface(kICookieStorageIID, (void**)&spCookieStorage);

    if (NS_SUCCEEDED(res) && spCookieStorage)
	return spCookieStorage->GetCookie(inCookieURL, inOutCookieBuffer, inOutCookieSize);

    return res;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginManager::SetCookie
//=--------------------------------------------------------------------------=
// params:  inCookieURL        URL string store cookie with.
//	    inCookieBuffer     buffer containing cookie data.
//	    inCookieSize       specifies  size of cookie data.
//
// notes:
//
JD_METHOD
CNSAdapter_PluginManager::SetCookie(const char* inCookieURL, const void* inCookieBuffer, JDUint32 inCookieSize)
{
    TRACE("CNSAdapter_PluginManager::SetCookie\n");
    
    if (m_pPluginManager == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsICookieStorage> spCookieStorage;
    nsresult res = m_pPluginManager->QueryInterface(kICookieStorageIID, (void**)&spCookieStorage);

    if (NS_SUCCEEDED(res) && spCookieStorage)
	res = spCookieStorage->SetCookie(inCookieURL, inCookieBuffer, inCookieSize);
    return res;
}
