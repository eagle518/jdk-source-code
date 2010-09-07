/*
 * @(#)CNS4Adapter_PluginManager.cpp	1.11 03/01/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginManager.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CNS4Adapter_PluginManager.cpp: implementation of the CNS4Adapter_PluginManager 
//
// This is the dummy plugin manager that interacts with the 5.0 plugin.
//

#include "StdAfx.h"

#ifdef NO_XEMBED_MOZILLA
#include "npapi_XEmbed.h"
#else
#include "npapi.h"
#endif	// NO_XEMBED_MOZILLA

#include "IPluginManager.h"
#include "IPluginInstance.h"
#include "IPluginInstancePeer.h"

#include "INS4AdapterPeer.h"
#include "CNS4Adapter_PluginInstancePeer.h"
#include "CNS4Adapter_PluginManager.h"
#include "Debug.h"


//=--------------------------------------------------------------------------=
// Global variables
//=--------------------------------------------------------------------------=

static JD_DEFINE_IID(jIPluginInstanceIID, IPLUGININSTANCE_IID);
extern "C" JDresult JDResultFromNPError(int err);



//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginManager::CNS4Adapter_PluginManager
//=--------------------------------------------------------------------------=
//
CNS4Adapter_PluginManager::CNS4Adapter_PluginManager(INS4AdapterPeer* peer) 
    : m_pINS4AdapterPeer(NULL)
{
    TRACE("CNS4Adapter_PluginManager::CNS4Adapter_PluginManager\n");

    // Set reference count to 0.
    JD_INIT_REFCNT();

    if (peer != NULL)
    {
	m_pINS4AdapterPeer = peer;
	m_pINS4AdapterPeer->AddRef();
    }
}


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginManager::~CNS4Adapter_PluginManager
//=--------------------------------------------------------------------------=
//
CNS4Adapter_PluginManager::~CNS4Adapter_PluginManager(void) 
{
    TRACE("CNS4Adapter_PluginManager::~CNS4Adapter_PluginManager\n");

    if (m_pINS4AdapterPeer != NULL)
    {
	m_pINS4AdapterPeer->Release();
	m_pINS4AdapterPeer = NULL;
    }
}


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginManager::GetURL
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginManager::GetURL(ISupports* inst, 
				  const char* url, 
				  const char* target,
				  IPluginStreamListener* streamListener,
				  const char* altHost,
				  const char* referrer,
				  JDBool forceJSEnabled)
{
    TRACE("CNS4Adapter_PluginManager::GetURL\n");

    ASSERT(m_pINS4AdapterPeer != NULL);
    ASSERT(inst != NULL);

    if (altHost != NULL || referrer != NULL || forceJSEnabled != JD_FALSE) 
        return NPERR_INVALID_PARAM;

    JDSmartPtr<IPluginInstance> spPluginInstance;
    IPluginInstancePeer* pPluginInstancePeer;

    if (JD_FAILED(inst->QueryInterface(jIPluginInstanceIID, (void**) &spPluginInstance)))
        return JD_ERROR_FAILURE;

    if (JD_FAILED(spPluginInstance->GetPeer(&pPluginInstancePeer)))
        return JD_ERROR_FAILURE;

    CNS4Adapter_PluginInstancePeer* pInstancePeer = (CNS4Adapter_PluginInstancePeer*)pPluginInstancePeer;
    NPP npp = pInstancePeer->GetNPPInstance();

    NPError err;
    // Call the correct PostURL* function.
    // This is determinded by checking notifyData.
    if (streamListener == NULL) 
    {
        err = m_pINS4AdapterPeer->NPN_GetURL(npp, url, target);
    } else 
    {
        err = m_pINS4AdapterPeer->NPN_GetURLNotify(npp, url, target, streamListener);
    }

    if (pPluginInstancePeer)
	pPluginInstancePeer->Release();
    
    return JDResultFromNPError(err);
}

JD_METHOD
CNS4Adapter_PluginManager::FindProxyForURL(const char* url, char* *result)
{
    return JD_ERROR_NOT_IMPLEMENTED;
}

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginManager::UserAgent
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginManager::UserAgent(const char* *result)
{
    TRACE("CNS4Adapter_PluginManager::UserAgent\n");

    ASSERT(m_pINS4AdapterPeer != NULL);

    if (result != NULL && m_pINS4AdapterPeer)
	*result = m_pINS4AdapterPeer->NPN_UserAgent(NULL);

    return JD_OK;
}
	
JD_METHOD 
CNS4Adapter_PluginManager::GetValue(JDPluginManagerVariable variable, void* value)
{
    TRACE("CNS4Adapter_PluginManager::GetValue\n");

    NPError err = NPERR_GENERIC_ERROR;
    ASSERT(m_pINS4AdapterPeer != NULL);
#ifdef XP_UNIX
    NPNVariable npv;
    switch (variable) {
    case JDPluginManagerVariable_XDisplay:
        npv = NPNVxDisplay;
        break;
    case JDPluginManagerVariable_XtAppContext:
        npv = NPNVxtAppContext;
        break;
    case JDPluginManagerVariable_SupportsXEmbed:
        npv = NPNVSupportsXEmbedBool;
        break;
    default:
        break;
    }

    if (m_pINS4AdapterPeer)
	err = m_pINS4AdapterPeer->NPN_GetValue(NULL, npv, value);
#endif
    return  JDResultFromNPError(err);
}
//=--------------------------------------------------------------------------=
// ISupports functions
//=--------------------------------------------------------------------------=
JD_IMPL_ISUPPORTS1(CNS4Adapter_PluginManager, IPluginManager);
