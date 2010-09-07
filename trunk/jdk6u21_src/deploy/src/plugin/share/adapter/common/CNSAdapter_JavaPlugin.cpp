/*
 * @(#)CNSAdapter_JavaPlugin.cpp	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_JavaPlugin.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_JavaPlugin.cpp: Implementation of adapter for nsIPluginInstance
//
#include "StdAfx.h"
#include "IPluginInstance.h"
#include "IPluginInstancePeer.h"
#include "IJVMPluginInstance.h"
#include "nsIPluginInstance.h"
#include "nsIPluginInstancePeer.h"
#include "nsIJVMPluginInstance.h"
#include "CNSAdapter_JavaPlugin.h"
#include "CNSAdapter_PluginInstancePeer.h"
#include "CMap.h"
#include "Debug.h"

// Netscape iid
static NS_DEFINE_IID(kIPluginInstanceIID, NS_IPLUGININSTANCE_IID);
static NS_DEFINE_IID(kIJVMPluginInstanceIID, NS_IJVMPLUGININSTANCE_IID);
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

// Plugin iid
static JD_DEFINE_IID(jIPluginInstanceIID, IPLUGININSTANCE_IID);
static JD_DEFINE_IID(jIJVMPluginInstanceIID, IJVMPLUGININSTANCE_IID);
static JD_DEFINE_IID(jISupportsIID, ISUPPORTS_IID);

// pluginMap stores the mapping between CNSAdapter_JavaPlugin and CJavaPlugin
// object. This map is used in liveconnect dispatch, java plug-in code only knows 
// CJavaPlugin, however, browser should only know CNSAdapter_JavaPlugin and 
// this is the exact reason of why we need to have this
CMap<void*, void*>  pluginMap;

//nsISupports
NS_IMPL_ISUPPORTS2(CNSAdapter_JavaPlugin, nsIPluginInstance, nsIJVMPluginInstance)

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::CNSAdapter_JavaPlugin
//=--------------------------------------------------------------------------=
//
// notes :
//
CNSAdapter_JavaPlugin::CNSAdapter_JavaPlugin(IPluginInstance* pJavaPlugin) : 
  m_pJavaPlugin(pJavaPlugin), m_peer(NULL)
{
    TRACE("CNSAdapter_JavaPlugin::CNSAdapter_JavaPlugin\n");

    NS_INIT_REFCNT();

    if (m_pJavaPlugin)
	m_pJavaPlugin->AddRef();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::~CNSAdapter_JavaPlugin
//=--------------------------------------------------------------------------=
//
// notes :
//
CNSAdapter_JavaPlugin::~CNSAdapter_JavaPlugin()
{
    TRACE("CNSAdapter_JavaPlugin::~CNSAdapter_JavaPlugin\n");

    if (m_pJavaPlugin)
	m_pJavaPlugin->Release();

    if (m_peer)
	m_peer->Release();

}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::Initialize
//=--------------------------------------------------------------------------=
// params: nsIPluginInstancePeer* peer 
//         the PluginInstancePeer is an object in the browser side to get information
//	   about plugin and get some service done
// return: NS_OK if call succeed
//
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::Initialize(nsIPluginInstancePeer* peer)
{
    TRACE("CNSAdapter_JavaPlugin::Initialize\n");
    if (peer == NULL || m_pJavaPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    m_peer = peer;
    m_peer->AddRef();

    JDSmartPtr<IPluginInstancePeer> spPluginInstancePeerAdapter(new CNSAdapter_PluginInstancePeer(peer));

    if (spPluginInstancePeerAdapter == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    nsresult res = m_pJavaPlugin->Initialize(spPluginInstancePeerAdapter);

    // Store the CNSAdapter_JavaPlugin and CPluginInstancePeerAdapter into the map
    // The reason is for liveconnect dispatch
    if (NS_SUCCEEDED(res))
	pluginMap.InsertElement((void*)m_pJavaPlugin, (void*)this);
    
    return res;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::GetPeer
//=--------------------------------------------------------------------------=
// params: nsIPluginInstancePeer** resultingPeer
//	    contains the result of peer associated with plugin
// return: NS_OK if call succeed
//
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::GetPeer(nsIPluginInstancePeer* *resultingPeer)
{
    TRACE("CNSAdapter_JavaPlugin::GetPeer\n");
    if (resultingPeer == NULL || m_pJavaPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    *resultingPeer = m_peer;

    m_peer->AddRef();

    return NS_OK;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::Start
//=--------------------------------------------------------------------------=
// This call will start Java Plug-in instance
//
// return: NS_OK if call succeed
//
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::Start()
{
    TRACE("CNSAdapter_JavaPlugin::Start\n");
    if (m_pJavaPlugin == NULL)
	return NS_ERROR_NULL_POINTER;
    
    nsresult res = NS_OK;
    res = m_pJavaPlugin->Start();
    
    if (NS_SUCCEEDED(res) && pluginMap.FindElement((void*)m_pJavaPlugin) == NULL)
        pluginMap.InsertElement((void*)m_pJavaPlugin, (void*)this);

    return res;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::Stop
//=--------------------------------------------------------------------------=
// This call will stop Java Plug-in instance
//
// return: NS_OK if call succeed
//
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::Stop()
{
    TRACE("CNSAdapter_JavaPlugin::Stop\n");
    if (m_pJavaPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    nsresult res = m_pJavaPlugin->Stop();

    return res;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::Destroy
//=--------------------------------------------------------------------------=
// This will destroy Java Plug-in instance
//
//result - NS_OK if call succeed
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::Destroy()
{
    TRACE("CNSAdapter_JavaPlugin::Destroy\n");
    if (m_pJavaPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    nsresult res = m_pJavaPlugin->Destroy();

    // Remove from pluginMap when applet is destroyed
    pluginMap.InsertElement((void*)m_pJavaPlugin, NULL);

    return res;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::SetWindow
//=--------------------------------------------------------------------------=
// This call will set the plug-in window
// params: nsPluginWindow* window
//	    the plugin window structure
//  result - NS_OK if call succeed
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::SetWindow(nsPluginWindow* window)
{
    TRACE("CNSAdapter_JavaPlugin::SetWindow\n");
    if (m_pJavaPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pJavaPlugin->SetWindow((JDPluginWindow*)window);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::NewStream
//=--------------------------------------------------------------------------=
//
// notes :
//	
NS_METHOD
CNSAdapter_JavaPlugin::NewStream(nsIPluginStreamListener** listener)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::Print
//=--------------------------------------------------------------------------=
// param: nsPluginPrint* platformPrint
// return: NS_OK if call succeed
//
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::Print(nsPluginPrint* platformPrint)
{
    TRACE("CNSAdapter_JavaPlugin::Print\n");
    if (m_pJavaPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pJavaPlugin->Print((JDPluginPrint*)platformPrint);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::GetValue
//=--------------------------------------------------------------------------=
//
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::GetValue(nsPluginInstanceVariable variable, void* value)
{
    TRACE("CNSAdapter_JavaPlugin::GetValue\n");
    if (m_pJavaPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    JDPluginInstanceVariable variable2;
	
    switch(variable)
    {
    case nsPluginInstanceVariable_WindowlessBool:
	    variable2 = JDPluginInstanceVariable_WindowlessBool;
	    break;
    case nsPluginInstanceVariable_TransparentBool:
	    variable2 = JDPluginInstanceVariable_TransparentBool;
	    break;
    case nsPluginInstanceVariable_DoCacheBool:
	    variable2 = JDPluginInstanceVariable_DoCacheBool;
	    break;
    case nsPluginInstanceVariable_CallSetWindowAfterDestroyBool:
	    variable2 = JDPluginInstanceVariable_CallSetWindowAfterDestroyBool;
	    break;
    case nsPluginInstanceVariable_ScriptableInstance:
	    variable2 = JDPluginInstanceVariable_ScriptableInstance;
	    break;
    case nsPluginInstanceVariable_ScriptableIID:
	    variable2 = JDPluginInstanceVariable_ScriptableIID;
	    break; 
    default:
#ifdef NO_XEMBED_MOZILLA
	    if (variable == nsPluginInstanceVariable_NeedsXEmbed)
	        variable2 = JDPluginInstanceVariable_NeedsXEmbed;
            else
#endif	// NO_XEMBED_MOZILLA
	        return NS_ERROR_FAILURE;
    };

    return m_pJavaPlugin->GetValue(variable2, value);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::HandleEvent
//=--------------------------------------------------------------------------=
//
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::HandleEvent(nsPluginEvent* event, PRBool* handled)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}


//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::GetJavaObject
//=--------------------------------------------------------------------------=
// params: jobject a JNI object represent the Java object
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::GetJavaObject(jobject *result)
{
    TRACE("CNSAdapter_JavaPlugin::GetJavaObject\n");
    if (m_pJavaPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    JDSmartPtr<IJVMPluginInstance> spJVMPluginInstance;

    JDresult res = m_pJavaPlugin->QueryInterface(jIJVMPluginInstanceIID, (void**)&spJVMPluginInstance);

    if (JD_SUCCEEDED(res) && spJVMPluginInstance != NULL)
	return spJVMPluginInstance->GetJavaObject(result);
		
    return res;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JavaPlugin::GetText
//=--------------------------------------------------------------------------=
// params: const char** result
// notes :
//
NS_METHOD
CNSAdapter_JavaPlugin::GetText(const char** result)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
