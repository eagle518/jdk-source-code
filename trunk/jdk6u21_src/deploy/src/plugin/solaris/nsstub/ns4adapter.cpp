/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

////////////////////////////////////////////////////////////////////////////////
// Backward Adapter
// This acts as a adapter layer to allow 5.0 plugins work with the 4.0/3.0 
// browser.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// SECTION 1 - Includes
////////////////////////////////////////////////////////////////////////////////

//extern "C" {

// Mozilla changes: Compiler seems to complain if this is not here...
#include "string.h"
/*
#ifdef XP_UNIX
#define _UINT32
#define _INT32
#endif
*/
#include "StdAfx.h"
#include "npapi.h"
#include "npupp.h"
#include "dlfcn.h"
#include "IPluginManager.h"
#include "INS4AdapterPeer.h"
#include "INS4AdapterPeerInit.h"
#include "IPlugin.h"
#include "IPluginInstance.h"
#include "INS4PluginInstance.h"
#include "IPluginInstancePeer.h"
#include "IPluginStream.h"
#include "IPluginStreamInfo.h"
#include "IPluginServiceProvider.h"
#include "CNS4Adapter_PluginStreamInfo.h"
#include "CNS4Adapter_PluginManager.h"
#include "CNS4Adapter_PluginInstancePeer.h"
#include "CNS4Adapter_PluginServiceProvider.h"
#include "CNS4AdapterPeer.h"
#include "pluginversion.h"
#include "IUniqueIdentifier.h"
#include "IUnixService.h"
#include "CNS4Adapter_UnixService.h"

#define UNUSED(x) x=x

static JD_DEFINE_IID(jINS4AdapterPeerInitIID, IID_INS4AdapterPeerInit);
static JD_DEFINE_IID(jIPluginIID, IPLUGIN_IID);
static JD_DEFINE_IID(jIPluginInstanceIID, IPLUGININSTANCE_IID);
static JD_DEFINE_IID(jINS4PluginInstanceIID, INS4PLUGININSTANCE_IID);
static JD_DEFINE_IID(jIUniqueIdentifierIID, UNIQUE_IDENTIFIER_IID);
//////////////////////////////////////////////////////////////////////////////

#ifdef XP_UNIX
#define TRACE(foo) trace(foo)
#endif

#ifdef XP_MAC
#undef assert
#define assert(cond)
#endif

//#if defined(__cplusplus)
//extern "C" {
//#endif

////////////////////////////////////////////////////////////////////////////////
// SECTION 1 - Includes
////////////////////////////////////////////////////////////////////////////////

#if defined(XP_UNIX) || defined(XP_MAC)
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#else
#include <windows.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// SECTION 2 - Global Variables
////////////////////////////////////////////////////////////////////////////////

//
// thePlugin and thePluginManager are used in the life of the plugin.
//
// These two will be created on NPP_Initialize and destroyed on NPP_Shutdown.
//
IPluginManager* thePluginManager = NULL;
IPlugin* thePlugin = NULL;
INS4AdapterPeer* theAdapterPeer = NULL;

extern JDresult LoadNSCore(void* *jpinsp);
extern void     UnloadNSCore(void* jpinsp);
static void *libjpinsp = NULL;


// mapping from NPError to nsresult
JDresult fromNPError[] = {
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

static NPError
JPI_GetFactory(IPluginManager* pluginManager, IFactory* *theFactory)
{
    JDresult rv = JD_OK;
  
    rv = LoadNSCore(&libjpinsp);

    if (rv == JD_OK) {
        JDSmartPtr<IPluginServiceProvider> spPluginServiceProvider = new 
            CPluginServiceProvider(pluginManager);
        
        IUnixService* us = new CNS4Adapter_UnixService;
        if (spPluginServiceProvider != NULL) {
            JD_METHOD (*createPluginFactory)(ISupports* sm, 
                                             IUnixService* us,
                                             IFactory* *res);
            createPluginFactory = (JD_METHOD (*)(ISupports*, IUnixService*, IFactory**))
                dlsym(libjpinsp, "createPluginFactory");

            rv = createPluginFactory(spPluginServiceProvider, us,
                                     theFactory);
        }
        else {
            rv = JD_ERROR_OUT_OF_MEMORY;
        }
    }
    
    return (NPError)rv;
}

static void
JPI_RemoveFactory(IFactory* pFactory) {
    pFactory->Release();

    UnloadNSCore(libjpinsp);
    libjpinsp = NULL;
    return;
}
       
////////////////////////////////////////////////////////////////////////////////
// SECTION 4 - API Shim Plugin Implementations
// Glue code to the 5.0x Plugin.
//
// Most of the NPP_* functions that interact with the plug-in will need to get 
// the instance peer from npp->pdata so it can get the plugin instance from the
// peer. Once the plugin instance is available, the appropriate 5.0 plug-in
// function can be called:
//          
//  CPluginInstancePeer* peer = (CPluginInstancePeer* )instance->pdata;
//  nsIPluginInstance* inst = peer->GetUserInstance();
//  inst->NewPluginAPIFunction();
//
// Similar steps takes place with streams.  The stream peer is stored in NPStream's
// pdata.  Get the peer, get the stream, call the function.
//

////////////////////////////////////////////////////////////////////////////////
// UNIX-only API calls
////////////////////////////////////////////////////////////////////////////////

#ifdef XP_UNIX
NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value);


char* NPP_GetMIMEDescription(void)
{
  return (char *) PLUGIN_MIMETABLE;
}

/*
 * NPP_GetValue is a special function for Unix, it plays two
 * roles. One is to return the description of plugin (not 
 * specific to any instances) and the other is to return 
 * the information about a specific plugin instance.
 * Assumptions made by the function: for the previous role,
 * we always assume that the return value is a string, this is
 * why we use strdup. For the other role, we just return a boolean
 *
 */ 
NPError
NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
    //fprintf(stderr, "MIME description\n");
    NPError err = NPERR_GENERIC_ERROR;

    if (thePlugin == NULL) {
        JDSmartPtr<IPluginManager> pm = new 
            CNS4Adapter_PluginManager(NULL);
        IFactory* pFactory;
        err = JPI_GetFactory(pm, (IFactory**)&pFactory);
       
        if (err != NPERR_NO_ERROR)
            return err;

        JDresult rv = pFactory->QueryInterface(jIPluginIID, (void**)&thePlugin);

        if (JD_SUCCEEDED(rv)) {
            rv = thePlugin->GetValue((JDPluginVariable)variable, value);
            if (JD_SUCCEEDED(rv)) {
                char* str;
                str = *((char**)value);
                *((char**)value) = strdup((const char*)str);
            }

            thePlugin->Release();
            thePlugin = NULL;
            pFactory->Release();

            err = (NPError)rv;
        }
        JPI_RemoveFactory(pFactory);
    }
       
    if (err != NPERR_NO_ERROR && instance != NULL) {
        JDSmartPtr<IPluginInstance> spPluginInstance = 
            (IPluginInstance*)instance->pdata;
        err = (NPError)spPluginInstance->GetValue((JDPluginInstanceVariable)variable, value);
    }
        
    return err;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_SetValue:
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError
NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
    UNUSED(instance);
    UNUSED(variable);
    UNUSED(value);
    return NPERR_GENERIC_ERROR; // nothing to set
}

#endif // XP_UNIX

// Internal function used by stubs.c to pass Navigtor func pointers
extern "C" {
NPError
NPP_PeerInitialize(NPNetscapeFuncs* pNavigatorFuncs)
{
    if (theAdapterPeer == NULL) {
        theAdapterPeer = new CNS4AdapterPeer;
        if (theAdapterPeer == NULL)
            return NPERR_OUT_OF_MEMORY_ERROR; 
        theAdapterPeer->AddRef();
    }
    JDSmartPtr<INS4AdapterPeerInit> spAdapterPeerInit;
    if(theAdapterPeer->QueryInterface(jINS4AdapterPeerInitIID,
                                      (void**)&spAdapterPeerInit) == JD_OK) {
        spAdapterPeerInit->Initialize(pNavigatorFuncs);
    }
 
    return NPERR_NO_ERROR;
}
}
//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_Initialize:
// Provides global initialization for a plug-in, and returns an error value. 
//
// This function is called once when a plug-in is loaded, before the first instance
// is created. thePluginManager and thePlugin are both initialized.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError
NPP_Initialize(void)
{
    //    TRACE("NPP_Initialize\n");
    // Only call initialize the plugin if it hasn't been created.
    // This could happen if GetJavaClass() is called before
    // NPP Initialize.
    if (thePluginManager == NULL) {
        // Create the plugin manager and plugin classes.
        thePluginManager = new CNS4Adapter_PluginManager(theAdapterPeer);
        if ( thePluginManager == NULL ) 
            return NPERR_OUT_OF_MEMORY_ERROR;  
        thePluginManager->AddRef();
    }
   
    JDresult rv = JD_ERROR_FAILURE;
    if (thePlugin == NULL) {
        JDSmartPtr<IFactory> spFactory;
        if (JPI_GetFactory(thePluginManager, (IFactory**)&spFactory) 
            == NPERR_NO_ERROR) {
            rv = spFactory->QueryInterface(jIPluginIID, (void**)&thePlugin);
      
            if (JD_SUCCEEDED(rv))
                thePlugin->Initialize();
        }
    }
       
    return (NPError) rv;	
}
//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_GetJavaClass:
// New in Netscape Navigator 3.0. 
// 
// NPP_GetJavaClass is called during initialization to ask your plugin
// what its associated Java class is. If you don't have one, just return
// NULL. Otherwise, use the javah-generated "use_" function to both
// initialize your class and return it. If you can't find your class, an
// error will be signalled by "use_" and will cause the Navigator to
// complain to the user.
//+++++++++++++++++++++++++++++++++++++++++++++++++

jref
NPP_GetJavaClass(void)
{
    // Only call initialize the plugin if it hasn't been `d.
    /*  if (thePluginManager == NULL) {
        // Create the plugin manager and plugin objects.
        NPError result = CPluginManager::Create();	
        if (result) return NULL;
	    assert( thePluginManager != NULL );
        thePluginManager->AddRef();
        NP_CreatePlugin(thePluginManager, (nsIPlugin** )(&thePlugin));
        assert( thePlugin != NULL );
        }
        */
//	return thePlugin->GetJavaClass();
	return NULL;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_Shutdown:
// Provides global deinitialization for a plug-in. 
// 
// This function is called once after the last instance of your plug-in 
// is destroyed.  thePluginManager and thePlugin are delete at this time.
//+++++++++++++++++++++++++++++++++++++++++++++++++

void
NPP_Shutdown(void)
{
//	TRACE("NPP_Shutdown\n");
    if (thePluginManager)  {
		thePluginManager->Release();
		thePluginManager = NULL;
	}

    if (theAdapterPeer) {
        theAdapterPeer->Release();
        theAdapterPeer = NULL;
    }

	if (thePlugin)
	{
		thePlugin->Shutdown();
		thePlugin->Release();
        JPI_RemoveFactory(thePlugin);
        thePlugin = NULL;
    }
    
    return;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_New:
// Creates a new instance of a plug-in and returns an error value. 
// 
// A plugin instance peer and instance peer is created.  After
// a successful instansiation, the peer is stored in the plugin
// instance's pdata.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError 
NPP_New(NPMIMEType pluginType,
	NPP instance,
	uint16 mode,
	int16 argc,
	char* argn[],
	char* argv[],
	NPSavedData* saved)
{
//    TRACE("NPP_New\n");
    UNUSED(mode);
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    if (thePlugin == NULL)
        return NPERR_GENERIC_ERROR;

    instance->pdata = NULL;

    // Create a new plugin instance and start it
    JDSmartPtr<IPluginInstance> spPluginInstance;
    thePlugin->CreateInstance(NULL, jIPluginInstanceIID, (void**)&spPluginInstance);

    if (spPluginInstance == NULL) {
        return NPERR_OUT_OF_MEMORY_ERROR;
    } 
    
    // Create a new plugin instance peer,
    // XXX - Since np_instance is not implemented in the 4.0x browser, I
    // XXX - had to save the plugin parameter in the peer class.
    // XXX - Ask Warren about np_instance.
    JDSmartPtr<IPluginInstancePeer> spPeer = 
        new CNS4Adapter_PluginInstancePeer(theAdapterPeer,
                                           instance, (JDPluginMimeType)pluginType, 
                                           (JDUint16)argc, 
                                           (const char** )argn, (const char** )argv);

    if (spPeer == NULL)
        return NPERR_OUT_OF_MEMORY_ERROR;

	JDSmartPtr<IUniqueIdentifier> spInst;
	if(JD_SUCCEEDED(spPluginInstance->QueryInterface(jIUniqueIdentifierIID, (void**)&spInst))) {
		long id = 0;
		if(NULL != saved) {
			id = saved->len;
			theAdapterPeer->NPN_MemFree(saved);
		}
		spInst->SetUniqueId(id);
	}

    spPluginInstance->Initialize(spPeer);
    spPluginInstance->Start();
    // Set the user instance and store the peer in npp->pdata.
    IPluginInstance* pPluginInstance = NULL;
    spPluginInstance.CopyTo(&pPluginInstance);

    instance->pdata = (void*)pPluginInstance;

    return NPERR_NO_ERROR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_Destroy:
// Deletes a specific instance of a plug-in and returns an error value. 
//
// The plugin instance peer and plugin instance are destroyed.
// The instance's pdata is set to NULL.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError 
NPP_Destroy(NPP instance, NPSavedData** save)
{
//    TRACE("NPP_Destroy\n");
    
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;
    
    JDSmartPtr<IPluginInstance> spPluginInstance = (IPluginInstance*)instance->pdata;	

    if (spPluginInstance == NULL)
        return NPERR_INVALID_PLUGIN_ERROR;

    JDSmartPtr<IUniqueIdentifier> spInst;
	if(JD_SUCCEEDED(spPluginInstance->QueryInterface(jIUniqueIdentifierIID, (void**)&spInst))) {
		*save = (NPSavedData*)theAdapterPeer->NPN_MemAlloc(sizeof(NPSavedData));
		long id;
		spInst->GetUniqueId(&id);
		(*save)->buf = NULL;
		(*save)->len = id;
	}
    spPluginInstance->Stop();
    spPluginInstance->Destroy();
    
    instance->pdata = NULL;
    
    return NPERR_NO_ERROR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_SetWindow:
// Sets the window in which a plug-in draws, and returns an error value. 
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError 
NPP_SetWindow(NPP instance, NPWindow* window)
{
//    TRACE("NPP_SetWindow\n");
    
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    IPluginInstance* pPluginInstance = (IPluginInstance* )instance->pdata;
    
    if( pPluginInstance == NULL )
        return NPERR_INVALID_PLUGIN_ERROR;

    return (NPError)pPluginInstance->SetWindow((JDPluginWindow* )window);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_NewStream:
// Notifies an instance of a new data stream and returns an error value. 
//
// Create a stream peer and stream.  If succesful, save
// the stream peer in NPStream's pdata.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError 
NPP_NewStream(NPP instance,
	      NPMIMEType type,
	      NPStream *stream, 
	      NPBool seekable,
	      uint16 *stype)
{
    // XXX - How do you set the fields of the peer stream and stream?
    // XXX - Looks like these field will have to be created since
    // XXX - We are not using np_stream.
   
//    TRACE("NPP_NewStream\n");

    UNUSED(seekable);
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;
		
    if (stream == NULL)
		return NPERR_INVALID_PLUGIN_ERROR;

	JDSmartPtr<IPluginStreamInfo> sPeer = new CNS4Adapter_PluginStreamInfo(theAdapterPeer,
                                                                           instance,
                                                                           stream,
                                                                           type,
                                                                           seekable);

    if (sPeer == NULL) return NPERR_OUT_OF_MEMORY_ERROR;
    IPluginStream* pluginStream = NULL;
    IPluginInstance* pluginInstance = (IPluginInstance*)instance->pdata;
    JDSmartPtr<INS4PluginInstance> spNS4PluginInstance;
    JDresult res = JD_OK;
    if (pluginInstance)
        res = pluginInstance->QueryInterface(jINS4PluginInstanceIID,
                                             (void**)&spNS4PluginInstance);

    if (JD_FAILED(res))
        return (NPError)res;

    res = spNS4PluginInstance->NewStream(sPeer, &pluginStream);

    if (JD_FAILED(res) || pluginStream == NULL) return NPERR_OUT_OF_MEMORY_ERROR;
    stream->pdata = (void*)pluginStream;
    JDPluginStreamType stype1;
    res = pluginStream->GetStreamType(&stype1);
    *stype = (uint16)stype1;

    return NPERR_NO_ERROR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_WriteReady:
// Returns the maximum number of bytes that an instance is prepared to accept
// from the stream. 
//+++++++++++++++++++++++++++++++++++++++++++++++++

int32 
NPP_WriteReady(NPP instance, NPStream *stream)
{
//    TRACE("NPP_WriteReady\n");

    if (instance == NULL)
        return -1;

    IPluginStream* theStream = (IPluginStream*)stream->pdata;
    if(theStream == 0 )
        return -1;
	
    return 8192;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_Write:
// Delivers data from a stream and returns the number of bytes written. 
//+++++++++++++++++++++++++++++++++++++++++++++++++

int32 
NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
//    TRACE("NPP_Write\n");

    if (instance == NULL)
        return -1;
	
    IPluginStream* theStream = (IPluginStream*)stream->pdata;
    
    if(theStream == 0 )
        return -1;
	
    JDint32 count = 0;

    if (JD_SUCCEEDED(theStream->Write((const char*)buffer, offset, len, &count)))
        return count;
    else
        return -1;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_DestroyStream:
// Indicates the closure and deletion of a stream, and returns an error value. 
//
// The stream peer and stream are destroyed.  NPStream's
// pdata is set to NULL.
//+++++++++++++++++++++++++++++++++++++++++++++++++

NPError 
NPP_DestroyStream(NPP instance, NPStream *stream, NPReason reason)
{
//    TRACE("NPP_DestroyStream\n");

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;
		
    IPluginStream* theStream = (IPluginStream*) stream->pdata;
    if( theStream == 0 )
        return NPERR_GENERIC_ERROR;
	
    theStream->Release();
    stream->pdata = NULL;
    UNUSED(reason);
	
    return NPERR_NO_ERROR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_StreamAsFile:
// Provides a local file name for the data from a stream. 
//+++++++++++++++++++++++++++++++++++++++++++++++++

void 
NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
//	TRACE("NPP_StreamAsFile\n");

	if (instance == NULL)
		return;
		
	IPluginStream* theStream = (IPluginStream*) stream->pdata;
	if( theStream == 0 )
		return;

	theStream->AsFile( fname );
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_Print:
//+++++++++++++++++++++++++++++++++++++++++++++++++

void 
NPP_Print(NPP instance, NPPrint* printInfo)
{
//    TRACE("NPP_Print\n");

    if(printInfo == NULL)   // trap invalid parm
        return;

	if (instance != NULL)
	{
		IPluginInstance* pluginInstance = (IPluginInstance*) instance->pdata;
		pluginInstance->Print((JDPluginPrint* ) printInfo );
	}
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
// NPP_URLNotify:
// Notifies the instance of the completion of a URL request. 
//+++++++++++++++++++++++++++++++++++++++++++++++++

void
NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
//   	TRACE("NPP_URLNotify\n");

    if( instance == NULL )
        return;
    IPluginInstance* pluginInstance = (IPluginInstance*) instance->pdata;
    JDSmartPtr<INS4PluginInstance> spNS4PluginInstance;
    JDresult res = JD_OK;
    if (pluginInstance)
        res = pluginInstance->QueryInterface(jINS4PluginInstanceIID,
                                            (void**)&spNS4PluginInstance);

    if (JD_FAILED(res) || spNS4PluginInstance == NULL)
        return;

    spNS4PluginInstance->URLNotify(url, NULL, (JDPluginReason)reason, notifyData);	
}
