/*
 * @(#)CNS4AdapterPeer.cpp	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CNS4AdapterPeer.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CNS4AdapterPeer.cpp : Implementation of CNS4AdapterPeer
//
// CNS4AdapterPeer is an abstraction of the Netsacpe Plug-in API on the
// browser side for Navigator 3/4.
//

#include "StdAfx.h"
#include "npupp.h"
#include "npapi.h"
#include "INS4AdapterPeer.h"
#include "INS4AdapterPeerInit.h"
#include "CNS4AdapterPeer.h"
#include "Debug.h"

/////////////////////////////////////////////////////////////////////////////
// CNS4AdapterPeer
JD_IMPL_ISUPPORTS2(CNS4AdapterPeer, INS4AdapterPeer, INS4AdapterPeerInit);

CNS4AdapterPeer::CNS4AdapterPeer()
{
    TRACE("CNS4AdapterPeer::CNS4AdapterPeer\n");

    m_pNavigatorFuncs = NULL;
}

JD_METHOD
CNS4AdapterPeer::Initialize(NPNetscapeFuncs *pNavigatorFuncs)
{
    TRACE("CNS4AdapterPeer::Initialize\n");

    ASSERT(pNavigatorFuncs != NULL);

    m_pNavigatorFuncs = pNavigatorFuncs;

    return JD_OK;
}

/*    NAVIGATOR Entry points    */

/* These entry points expect to be called from within the plugin.  The
   noteworthy assumption is that DS has already been set to point to the
   plugin's DLL data segment.  Don't call these functions from outside
   the plugin without ensuring DS is set to the DLLs data segment first,
   typically using the NP_LOADDS macro
*/

/* returns the major/minor version numbers of the Plugin API for the plugin
   and the Navigator
*/
JD_METHOD_(void) CNS4AdapterPeer::NPN_Version(int* plugin_major, 
                                              int* plugin_minor, 
                                              int* netscape_major, 
                                              int* netscape_minor)
{
    TRACE("CNS4AdapterPeer::NPN_Version\n");

    ASSERT(m_pNavigatorFuncs != NULL);
    ASSERT(plugin_major != NULL);
    ASSERT(plugin_minor != NULL);
    ASSERT(netscape_major != NULL);
    ASSERT(netscape_minor != NULL);

    *plugin_major   = NP_VERSION_MAJOR;
    *plugin_minor   = NP_VERSION_MINOR;
    *netscape_major = HIBYTE(m_pNavigatorFuncs->version);
    *netscape_minor = LOBYTE(m_pNavigatorFuncs->version);
}

/* causes the specified URL to be fetched and streamed in
*/
JD_METHOD_(NPError) CNS4AdapterPeer::NPN_GetURLNotify(NPP instance, 
					              const char *url, 
						      const char *target, 
						      void* notifyData)

{
    TRACE("CNS4AdapterPeer::NPN_GetURLNotify\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    int navMinorVers = m_pNavigatorFuncs->version & 0xFF;
    NPError err;
    if( navMinorVers >= NPVERS_HAS_NOTIFICATION ) {
	err = m_pNavigatorFuncs->geturlnotify(instance, url, target, notifyData);
    }
    else {
	err = NPERR_INCOMPATIBLE_VERSION_ERROR;
    }
    return err;
}


JD_METHOD_(NPError) CNS4AdapterPeer::NPN_GetURL(NPP instance, 
						const char *url, 
						const char *target)
{
    TRACE("CNS4AdapterPeer::NPN_GetURL\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    return m_pNavigatorFuncs->geturl(instance, url, target);
}


JD_METHOD_(NPError) CNS4AdapterPeer::NPN_PostURLNotify(NPP instance, 
						       const char* url, 
						       const char* window, 
						       uint32 len, 
						       const char* buf, 
						       NPBool file, 
						       void* notifyData)
{
    TRACE("CNS4AdapterPeer::NPN_PostURLNotify\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    int navMinorVers = m_pNavigatorFuncs->version & 0xFF;
    NPError err;
    if( navMinorVers >= NPVERS_HAS_NOTIFICATION ) {
	err = m_pNavigatorFuncs->posturlnotify(instance, url, window, len, buf, file, notifyData);
    }
    else {
	err = NPERR_INCOMPATIBLE_VERSION_ERROR;
    }
    return err;
}


JD_METHOD_(NPError) CNS4AdapterPeer::NPN_PostURL(NPP instance, 
						 const char* url, 
						 const char* window, 
						 uint32 len, 
						 const char* buf, 
						 NPBool file)
{
    TRACE("CNS4AdapterPeer::NPN_PostURL\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    return m_pNavigatorFuncs->posturl(instance, url, window, len, buf, file);
}


/* Requests that a number of bytes be provided on a stream.  Typically
   this would be used if a stream was in "pull" mode.  An optional
   position can be provided for streams which are seekable.
*/
JD_METHOD_(NPError) CNS4AdapterPeer::NPN_RequestRead(NPStream* stream, 
						     NPByteRange* rangeList)
{
    TRACE("CNS4AdapterPeer::NPN_RequestRead\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    return m_pNavigatorFuncs->requestread(stream, rangeList);
}


/* Creates a new stream of data from the plug-in to be interpreted
   by Netscape in the current window.
*/
JD_METHOD_(NPError) CNS4AdapterPeer::NPN_NewStream(NPP instance, 
						   NPMIMEType type, 
						   const char* target, 
						   NPStream** stream)
{
    TRACE("CNS4AdapterPeer::NPN_NewStream\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    int navMinorVersion = m_pNavigatorFuncs->version & 0xFF;
    NPError err;

    if( navMinorVersion >= NPVERS_HAS_STREAMOUTPUT ) {
	err = m_pNavigatorFuncs->newstream(instance, type, target, stream);
    }
    else {
	err = NPERR_INCOMPATIBLE_VERSION_ERROR;
    }
    return err;
}


/* Provides len bytes of data.
*/
JD_METHOD_(int32) CNS4AdapterPeer::NPN_Write(NPP instance, 
					     NPStream *stream,
					     int32 len, 
					     void *buffer)
{
    TRACE("CNS4AdapterPeer::NPN_Write\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    int navMinorVersion = m_pNavigatorFuncs->version & 0xFF;
    int32 result;

    if( navMinorVersion >= NPVERS_HAS_STREAMOUTPUT ) {
	result = m_pNavigatorFuncs->write(instance, stream, len, buffer);
    }
    else {
	result = -1;
    }
    return result;
}


/* Closes a stream object.  
reason indicates why the stream was closed.
*/
JD_METHOD_(NPError) CNS4AdapterPeer::NPN_DestroyStream(NPP instance, 
						       NPStream* stream, 
						       NPError reason)
{
    TRACE("CNS4AdapterPeer::NPN_DestroyStream\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    int navMinorVersion = m_pNavigatorFuncs->version & 0xFF;
    NPError err;

    if( navMinorVersion >= NPVERS_HAS_STREAMOUTPUT ) {
	err = m_pNavigatorFuncs->destroystream(instance, stream, reason);
    }
    else {
	err = NPERR_INCOMPATIBLE_VERSION_ERROR;
    }
    return err;
}


/* Provides a text status message in the Netscape client user interface
*/
JD_METHOD_(void) CNS4AdapterPeer::NPN_Status(NPP instance, 
					     const char *message)
{
    TRACE("CNS4AdapterPeer::NPN_Status\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    m_pNavigatorFuncs->status(instance, message);
}


/* returns the user agent string of Navigator, which contains version info
*/
JD_METHOD_(const char*) CNS4AdapterPeer::NPN_UserAgent(NPP instance)
{
    TRACE("CNS4AdapterPeer::NPN_UserAgent\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    return m_pNavigatorFuncs->uagent(instance);
}

/* allocates memory from the Navigator's memory space.  Necessary so that
   saved instance data may be freed by Navigator when exiting.
*/


JD_METHOD_(void*) CNS4AdapterPeer::NPN_MemAlloc(uint32 size)
{
    TRACE("CNS4AdapterPeer::NPN_MemAlloc\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    return m_pNavigatorFuncs->memalloc(size);
}


/* reciprocal of MemAlloc() above
*/
JD_METHOD_(void) CNS4AdapterPeer::NPN_MemFree(void* ptr)
{
    TRACE("CNS4AdapterPeer::NPN_MemFree\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    m_pNavigatorFuncs->memfree(ptr);
}

JD_METHOD_(uint32) CNS4AdapterPeer::NPN_MemFlush(uint32) 
{
    TRACE("CNS4AdapterPeer::NPN_MemFlush\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    return 0;
}
   
/* private function to Netscape.  do not use!
*/
JD_METHOD_(void) CNS4AdapterPeer::NPN_ReloadPlugins(NPBool reloadPages)
{
    TRACE("CNS4AdapterPeer::NPN_ReloadPlugins\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    m_pNavigatorFuncs->reloadplugins(reloadPages);
}

JD_METHOD_(JRIEnv*) CNS4AdapterPeer::NPN_GetJavaEnv(void)
{
    TRACE("CNS4AdapterPeer::NPN_GetJavaEnv\n");

    ASSERT(m_pNavigatorFuncs != NULL);

    return m_pNavigatorFuncs->getJavaEnv();
}

JD_METHOD_(jref) CNS4AdapterPeer::NPN_GetJavaPeer(NPP instance)
{
    TRACE("CNS4AdapterPeer::NPN_GetJavaPeer\n");
    ASSERT(m_pNavigatorFuncs != NULL);

    return m_pNavigatorFuncs->getJavaPeer(instance);
}


JD_METHOD_(NPError) CNS4AdapterPeer::NPN_GetValue(NPP instance,
                                                  NPNVariable variable,
                                                  void* value) {

    TRACE("CNS4AdapterPeer::NPN_GetValue\n");
    ASSERT(m_pNavigatorFuncs != NULL);
#ifdef XP_UNIX
    return m_pNavigatorFuncs->getvalue(instance, variable, value);
#else
    return NPERR_GENERIC_ERROR;
#endif
}
