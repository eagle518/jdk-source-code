/*
 * @(#)CNS4Adapter.cpp	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CNS4Adapter.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CNS4Adapter.cpp : Implementation of CNS4Adapter
//
// This acts as a adapter layer to allow OJI plugins work with the 4.0/3.0 
// browser.
//

#include "StdAfx.h"
#include "IPlugin.h"
#include "IPluginInstance.h"
#include "IPluginServiceProvider.h"
#include "IPluginStreamListener.h"
#include "IFactory.h"
#include "oji_clsid.h"
#include "npapi.h"
#include "npupp.h"
#include "INS4AdapterPeer.h"
#include "INS4AdapterPeerInit.h"
#include "INS4Adapter.h"
#include "INS4AdapterInit.h"
#include "IUniqueIdentifier.h"
#include "CNS4AdapterPeer.h"
#include "CNS4Adapter.h"
#include "CNS4Adapter_PluginManager.h"
#include "CNS4Adapter_PluginStreamInfo.h"
#include "CNS4Adapter_PluginInstancePeer.h"
#include "CNS4Adapter_PluginServiceProvider.h"

#include "clientLoadCOM.h"
#include "Debug.h"

//=--------------------------------------------------------------------------=
// Global Variables
//=--------------------------------------------------------------------------=
static JD_DEFINE_IID(jIPluginIID, IPLUGIN_IID);
static JD_DEFINE_IID(jIPluginInstanceIID, IPLUGININSTANCE_IID);
static JD_DEFINE_IID(jIUniqueIdentifierIID, UNIQUE_IDENTIFIER_IID);

const LPCSTR NSGetFactory_NAME = "NSGetFactory";

typedef JDresult (*NSGetFactoryProc)(IPluginServiceProvider* pPluginServiceProvider,
				     IFactory** factory);

// mapping from NPError to nsresult
static JDresult fromNPError[] = {
    JD_OK,                          // NPERR_NO_ERROR,
    JD_ERROR_FAILURE,               // NPERR_GENERIC_ERROR,
    JD_ERROR_FAILURE,               // NPERR_INVALID_INSTANCE_ERROR,
    JD_ERROR_NOT_INITIALIZED,       // NPERR_INVALID_FUNCTABLE_ERROR,
    JD_ERROR_FACTORY_NOT_LOADED,    // NPERR_MODULE_LOAD_FAILED_ERROR,
    JD_ERROR_OUT_OF_MEMORY,         // NPERR_OUT_OF_MEMORY_ERROR,
    JD_NOINTERFACE,                 // NPERR_INVALID_PLUGIN_ERROR,
    JD_ERROR_ILLEGAL_VALUE,         // NPERR_INVALID_PLUGIN_DIR_ERROR,
    JD_NOINTERFACE,                 // NPERR_INCOMPATIBLE_VERSION_ERROR,
    JD_ERROR_ILLEGAL_VALUE,         // NPERR_INVALID_PARAM,
    JD_ERROR_ILLEGAL_VALUE,         // NPERR_INVALID_URL,
    JD_ERROR_ILLEGAL_VALUE,         // NPERR_FILE_NOT_FOUND,
    JD_ERROR_FAILURE,               // NPERR_NO_DATA,
    JD_ERROR_FAILURE                // NPERR_STREAM_NOT_SEEKABLE,
};


//=--------------------------------------------------------------------------=
// JDResultFromNPError
//=--------------------------------------------------------------------------=
//
// JDResultFromNPError convert NPError into nsresult.
//
// @param err NPError code
// @return JDresult code
//
extern "C" JDresult JDResultFromNPError(int err)
{
    return fromNPError[err];
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::Initialize
//=--------------------------------------------------------------------------=
// Initialize Netscape 4.x adapter object with the function pointer table of the
// Netscape Plug-in API for the browser side.
//
// @param pNavigatorFuncs function pointer table
// @return NPError
//
NPError
CNS4Adapter::Initialize(NPNetscapeFuncs* pNavigatorFuncs)
{
    TRACE("CNS4Adapter::Initialize\n");

    ASSERT(pNavigatorFuncs != NULL);

    if (m_pINS4AdapterPeer != NULL)
	return NPERR_GENERIC_ERROR;

    JDSmartPtr<INS4AdapterPeer> spINS4AdapterPeer = new CNS4AdapterPeer;

    JDSmartPtr<INS4AdapterPeerInit> spINS4AdapterPeerInit;

    spINS4AdapterPeer->QueryInterface(IID_INS4AdapterPeerInit, 
				     (void**)&spINS4AdapterPeerInit);

    spINS4AdapterPeerInit->Initialize(pNavigatorFuncs);

    spINS4AdapterPeerInit->QueryInterface(IID_INS4AdapterPeer, 
					  (void**)&m_pINS4AdapterPeer);

    return NPERR_NO_ERROR;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::FinalRelease
//=--------------------------------------------------------------------------=
// FinalRelease is called when the NS4 adapter object is about to be 
// released and destroyed.
//
//
void 
CNS4Adapter::FinalRelease()
{
    TRACE("CNS4Adapter::FinalRelease\n");

    if (m_pINS4AdapterPeer != NULL)
    {
	m_pINS4AdapterPeer->Release();
	m_pINS4AdapterPeer = NULL;
    }
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_Initialize
//=--------------------------------------------------------------------------=
// Provides global initialization for a plug-in, and returns an error value. 
//
// This function is called once when a plug-in is loaded, before the first instance
// is created. m_pPluginManager and m_pPlugin are both initialized.
//
//
NPError
CNS4Adapter::NPP_Initialize(void)
{
    TRACE("CNS4Adapter::NPP_Initialize\n");

    ASSERT(m_pINS4AdapterPeer != NULL);

    // Only call initialize the plugin if it hasn't been created.
    // This could happen if GetJavaClass() is called before
    // NPP Initialize.  
    if (m_pPluginManager == NULL) 
    {
        // Create the plugin manager and plugin classes.
        m_pPluginManager = new CNS4Adapter_PluginManager(m_pINS4AdapterPeer);	
     
        if (m_pPluginManager == NULL) 
            return NPERR_OUT_OF_MEMORY_ERROR;  

        m_pPluginManager->AddRef();
    }

    JDresult error = JD_OK;  
    
    // On UNIX the plugin might have been created when calling NPP_GetMIMEType.
    if (m_pPlugin == NULL) 
    {
        // create IPlugin factory from ns4 adapter
	static NSGetFactoryProc pProc = NULL;

	if (pProc == NULL)
	{
	    // Load jpins32.dll
            HINSTANCE hDLL = NULL;
	    if (LoadPluginLibrary(JPINS_SERVER_NAME, &hDLL) != ERROR_SUCCESS)
		return (NPError)JD_ERROR_FAILURE;

		// Get the proc address of NSGetFactory(IPluginServiceProvider*, IFactory**);	
		pProc = (NSGetFactoryProc)
			GetProcAddress(hDLL, NSGetFactory_NAME );
                if (pProc == NULL)
                    return NPERR_GENERIC_ERROR;
	}

	JDSmartPtr<IFactory> spIFactory;
	JDSmartPtr<IPluginServiceProvider> spPluginServiceProvider(new CPluginServiceProvider(m_pPluginManager));
    
	if (spPluginServiceProvider == NULL)
	    return (NPError)JD_ERROR_OUT_OF_MEMORY;

	// Calling NSGetFactory to get a Factory object
	error = pProc(spPluginServiceProvider, &spIFactory);
    
	if (JD_SUCCEEDED(error) && spIFactory)
	{
	    // Query IPlugin
	    error = spIFactory->QueryInterface(jIPluginIID, (void**)&m_pPlugin);
	    if (JD_SUCCEEDED(error) && m_pPlugin)
		m_pPlugin->Initialize();
	}

    }
    return (NPError) error;	
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_Shutdown
//=--------------------------------------------------------------------------=
// Provides global deinitialization for a plug-in. 
// 
// This function is called once after the last instance of your plug-in 
// is destroyed.  m_pPluginManager and m_pPlugin are delete at this time.
//
void
CNS4Adapter::NPP_Shutdown(void)
{
    TRACE("CNS4Adapter::NPP_Shutdown\n");

    if (m_pPlugin)
    {
	m_pPlugin->Shutdown();
	m_pPlugin->Release();
	m_pPlugin = NULL;
    }

    if (m_pPluginManager)  {
	m_pPluginManager->Release();
	m_pPluginManager = NULL;
    }
    
    return;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_GetJavaClass:
//=--------------------------------------------------------------------------=
// NPP_GetJavaClass is called during initialization to ask your plugin
// what its associated Java class is. If you don't have one, just return
// NULL. Otherwise, use the javah-generated "use_" function to both
// initialize your class and return it. If you can't find your class, an
// error will be signalled by "use_" and will cause the Navigator to
// complain to the user.
//
jref
CNS4Adapter::NPP_GetJavaClass(void)
{
    TRACE("CNS4Adapter::NPP_GetJavaClass\n");

    // OJI Plug-in does not support JRI at all.
    return NULL;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_New
//=--------------------------------------------------------------------------=
// Creates a new instance of a plug-in and returns an error value. 
// 
// A plugin instance peer and instance peer is created.  After
// a successful instansiation, the peer is stored in the plugin
// instance's pdata.
//
NPError 
CNS4Adapter::NPP_New(NPMIMEType pluginType,
		     NPP instance,
		     JDUint16 mode,
		     int16 argc,
		     char* argn[],
		     char* argv[],
		     NPSavedData* saved)
{
    TRACE("CNS4Adapter::NPP_New\n");
    
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    instance->pdata = NULL;

    // Create a new plugin instance and start it.
    JDSmartPtr<IPluginInstance> spPluginInstance;
    m_pPlugin->CreateInstance(NULL, jIPluginInstanceIID, (void**)&spPluginInstance);

    if (spPluginInstance == NULL) 
        return NPERR_OUT_OF_MEMORY_ERROR;
    
    // Create a new plugin instance peer,
    // XXX - Since np_instance is not implemented in the 4.0x browser, I
    // XXX - had to save the plugin parameter in the peer class.
    // XXX - Ask Warren about np_instance.
    JDSmartPtr<IPluginInstancePeer>  spPeer = new CNS4Adapter_PluginInstancePeer(m_pINS4AdapterPeer, 
									         instance, 
									         (JDPluginMimeType)pluginType, 
									         (JDUint16)argc, (const char** )argn, 
									         (const char** )argv);

    if (spPeer == NULL) 
	return NPERR_OUT_OF_MEMORY_ERROR;
    
    JDSmartPtr<IUniqueIdentifier> spInst;
    if(JD_SUCCEEDED(spPluginInstance->QueryInterface(jIUniqueIdentifierIID, (void**)&spInst))) 
    { 
        long id = 0;
        if(NULL != saved) {
	    id = (long)saved->len;
	    m_pINS4AdapterPeer->NPN_MemFree(saved);
        }
        spInst->SetUniqueId(id);
    }
    // Init and startup plug-in instance
    spPluginInstance->Initialize(spPeer);
    spPluginInstance->Start();
    
    // Set the user instance and store the peer in npp->pdata.
    IPluginInstance* pPluginInstance = NULL;
    spPluginInstance.CopyTo(&pPluginInstance);
   
    instance->pdata = (void*)pPluginInstance;
    
    return NPERR_NO_ERROR;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_Destroy
//=--------------------------------------------------------------------------=
// Deletes a specific instance of a plug-in and returns an error value. 
//
// The plugin instance peer and plugin instance are destroyed.
// The instance's pdata is set to NULL.
//
NPError 
CNS4Adapter::NPP_Destroy(NPP instance, NPSavedData** save)
{
    TRACE("CNS4Adapter::NPP_Destroy\n");
    
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;
    
    JDSmartPtr<IPluginInstance> spPluginInstance = (IPluginInstance* )instance->pdata;

    if (spPluginInstance == NULL)
	return NPERR_INVALID_PLUGIN_ERROR;

    // Stop and destroy the plug-in instance
    JDSmartPtr<IUniqueIdentifier> spInst;
    if(JD_SUCCEEDED(spPluginInstance->QueryInterface(jIUniqueIdentifierIID, (void**)&spInst))) 
    { 
	*save = (NPSavedData*)m_pINS4AdapterPeer->NPN_MemAlloc(sizeof(NPSavedData));
        (*save)->buf = NULL;
        spInst->GetUniqueId((long*)&((*save)->len));
    }

    // Stop and destroy the plug-in instance
    spPluginInstance->Stop();
    spPluginInstance->Destroy();
    spPluginInstance->Release();

    instance->pdata = NULL;
    
    return NPERR_NO_ERROR;
}

//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_SetWindow
//=--------------------------------------------------------------------------=
// Sets the window in which a plug-in draws, and returns an error value. 
//
NPError 
CNS4Adapter::NPP_SetWindow(NPP instance, NPWindow* window)
{
    TRACE("CNS4Adapter::NPP_SetWindow\n");
    
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    JDSmartPtr<IPluginInstance> spPluginInstance = (IPluginInstance* )instance->pdata;
    
    if(spPluginInstance == NULL)
        return NPERR_INVALID_PLUGIN_ERROR;

    return (NPError)spPluginInstance->SetWindow((JDPluginWindow* ) window );
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_NewStream
//=--------------------------------------------------------------------------=
// Notifies an instance of a new data stream and returns an error value. 
//
// Create a stream peer and stream.  If succesful, save
// the stream peer in NPStream's pdata.
//
NPError 
CNS4Adapter::NPP_NewStream(NPP instance,
			   NPMIMEType type,
			   NPStream *stream, 
			   NPBool seekable,
			   JDUint16 *stype)
{
    TRACE("CNS4Adapter::NPP_NewStream\n");

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    if (stream == NULL)
	return NPERR_INVALID_PLUGIN_ERROR;
				
    JDSmartPtr<IPluginStreamListener> spStreamListener = (IPluginStreamListener*)stream->notifyData;
    stream->pdata = NULL;

    if (spStreamListener != NULL)
    {
	IPluginStreamInfo* 
	pInfo = new CNS4Adapter_PluginStreamInfo(m_pINS4AdapterPeer, instance, stream, 
						 type, seekable);
	stream->pdata = (void*) pInfo;
        JDresult err = spStreamListener->OnStartBinding(pInfo);
        if (JD_FAILED(err)) 
            return err;

        // Obtain stream type
        err = spStreamListener->GetStreamType((JDPluginStreamType*)stype);
        assert(err == JD_OK);
    }
    else
    {
        *stype = JDPluginStreamType_Normal;
    }

    return NPERR_NO_ERROR;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_WriteReady
//=--------------------------------------------------------------------------=
// Returns the maximum number of bytes that an instance is prepared to accept
// from the stream. 
//
int32 
CNS4Adapter::NPP_WriteReady(NPP instance, NPStream *stream)
{
    TRACE("CNS4Adapter::NPP_WriteReady\n");

    if (instance == NULL)
        return -1;

    if (stream == NULL)
	return -1;
	
    return 8192;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_Write
//=--------------------------------------------------------------------------=
// Delivers data from a stream and returns the number of bytes written. 
//
int32 
CNS4Adapter::NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
    TRACE("CNS4Adapter::NPP_Write\n");

    if (instance == NULL)
        return -1;
	
    if (stream == NULL)
	return -1;

    JDSmartPtr<IPluginStreamListener> spStreamListener = (IPluginStreamListener*)stream->notifyData;
    JDint32 count = 0;

    if (spStreamListener != NULL)
    {
	JDSmartPtr<IPluginStreamInfo> spPInfo = (IPluginStreamInfo*)stream->pdata;    
        JDresult err = spStreamListener->OnDataAvailable(spPInfo, (JDint32)offset, (JDint32)len, (const char*)buffer, &count);

        if (err != JD_OK) 
	    return -1;
    }

    return count;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_DestroyStream
//=--------------------------------------------------------------------------=
// Indicates the closure and deletion of a stream, and returns an error value. 
//
// The stream peer and stream are destroyed.  NPStream's
// pdata is set to NULL.
//
NPError 
CNS4Adapter::NPP_DestroyStream(NPP instance, NPStream *stream, NPReason reason)
{
    TRACE("CNS4Adapter::NPP_DestroyStream\n");

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;
	
    if (stream == NULL)
        return NPERR_INVALID_PLUGIN_ERROR;

    JDSmartPtr<IPluginStreamListener> spStreamListener = (IPluginStreamListener*)stream->notifyData;

    if (spStreamListener != NULL)  
        spStreamListener->OnStopBinding((IPluginStreamInfo*)stream->pdata, (JDPluginReason)reason);

    stream->pdata = NULL;

    return NPERR_NO_ERROR;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_StreamAsFile
//=--------------------------------------------------------------------------=
// Provides a local file name for the data from a stream. 
//
void 
CNS4Adapter::NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
    TRACE("CNS4Adapter::NPP_StreamAsFile\n");

    if (instance == NULL)
	return;
		
    JDSmartPtr<IPluginStreamListener> spStreamListener = (IPluginStreamListener*)stream->notifyData;
    JDSmartPtr<IPluginStreamInfo> spPInfo = (IPluginStreamInfo*)stream->pdata;

    if (spStreamListener != NULL)
        spStreamListener->OnFileAvailable(spPInfo, fname);
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_Print
//=--------------------------------------------------------------------------=
void 
CNS4Adapter::NPP_Print(NPP instance, NPPrint* printInfo)
{
    TRACE("CNS4Adapter::NPP_Print\n");

    if(printInfo == NULL)   // trap invalid parm
        return;

    if (instance != NULL)
    {
	JDSmartPtr<IPluginInstance> spPluginInstance = (IPluginInstance*) instance->pdata;

	if (spPluginInstance != NULL)
	    spPluginInstance->Print((JDPluginPrint* ) printInfo);
    }
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_URLNotify
//=--------------------------------------------------------------------------=
// Notifies the instance of the completion of a URL request. 
//
void
CNS4Adapter::NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
    TRACE("CNS4Adapter::NPP_URLNotify\n");

    if (instance != NULL)
    {
        JDSmartPtr<IPluginStreamListener> spStreamListener = (IPluginStreamListener*)notifyData;

        if (spStreamListener != NULL)
            spStreamListener->Release();
    }
}


//=--------------------------------------------------------------------------=
// CNS4Adapter::NPP_HandleEvent 
//=--------------------------------------------------------------------------=
// Mac-only, but stub must be present for Windows
// Delivers a platform-specific event to the instance. 
//
int16
CNS4Adapter::NPP_HandleEvent(NPP instance, void* event)
{
    TRACE("CNS4Adapter::NPP_HandleEvent\n");

    // Not implemented
    return 0;
}

