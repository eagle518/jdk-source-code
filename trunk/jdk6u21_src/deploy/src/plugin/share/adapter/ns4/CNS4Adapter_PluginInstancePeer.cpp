/*
 * @(#)CNS4Adapter_PluginInstancePeer.cpp	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInstancePeer.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CNS4Adapter_PluginInstancePeer.cpp: implementation for the 
//				      CNS4Adapter_PluginInstancePeer class.
//
// This is the dummy instance peer that interacts with the 5.0 plugin.
// In order to do LiveConnect, the class subclasses nsILiveConnectPluginInstancePeer.
//

#include "StdAfx.h"
#include "npapi.h"
#include "IPluginInstancePeer.h"
#include "IPluginTagInfo.h"

#include "INS4AdapterPeer.h"
#include "CNS4Adapter_PluginInstancePeer.h"

#include "Debug.h"

extern "C" JDresult JDResultFromNPError(int err);

//=--------------------------------------------------------------------------=
// ISupports interface
//=--------------------------------------------------------------------------=
//
JD_IMPL_ISUPPORTS2(CNS4Adapter_PluginInstancePeer, IPluginInstancePeer, IPluginTagInfo);

//=--------------------------------------------------------------------------=
// Global variables
//=--------------------------------------------------------------------------=

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInstancePeer::CNS4Adapter_PluginInstancePeer
//=--------------------------------------------------------------------------=
//
CNS4Adapter_PluginInstancePeer::CNS4Adapter_PluginInstancePeer(INS4AdapterPeer* peer,
							       NPP npp,
							       JDPluginMimeType typeString, 
							       JDUint16 attr_cnt, 
							       const char** attr_list, 
							       const char** val_list)
    : m_npp(npp), m_typeString(typeString), 
      m_attribute_cnt((JDUint16)0),
      m_attribute_list(NULL), m_values_list(NULL), m_pINS4AdapterPeer(NULL)
{
    TRACE("CNS4Adapter_PluginInstancePeer::CNS4Adapter_PluginInstancePeer\n");

    ASSERT(peer != NULL);

    // Set the reference count to 0.
    JD_INIT_REFCNT();

    m_pINS4AdapterPeer = peer;

    if (m_pINS4AdapterPeer != NULL)
	m_pINS4AdapterPeer->AddRef();

    m_attribute_list = (char**) m_pINS4AdapterPeer->NPN_MemAlloc(attr_cnt * sizeof(const char*));
    m_values_list = (char**) m_pINS4AdapterPeer->NPN_MemAlloc(attr_cnt * sizeof(const char*));

    int j = 0;
    if (m_attribute_list != NULL && m_values_list != NULL) 
    {
	for (int i = 0; i < attr_cnt; i++)   
	{
	    if (attr_list[i] != NULL && val_list[i] != NULL)
	    {
		m_attribute_list[j] = (char*) m_pINS4AdapterPeer->NPN_MemAlloc(strlen(attr_list[i]) + 1);
		if (m_attribute_list[j] != NULL)
		    strcpy(m_attribute_list[j], attr_list[i]);

		m_values_list[j] = (char*) m_pINS4AdapterPeer->NPN_MemAlloc(strlen(val_list[i]) + 1);
		if (m_values_list[j] != NULL)
		    strcpy(m_values_list[j], val_list[i]);

                j++;
            }
	}
    }
    m_attribute_cnt = j;
    
    
}       


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInstancePeer::~CNS4Adapter_PluginInstancePeer
//=--------------------------------------------------------------------------=
//
CNS4Adapter_PluginInstancePeer::~CNS4Adapter_PluginInstancePeer(void) 
{
    TRACE("CNS4Adapter_PluginInstancePeer::~CNS4Adapter_PluginInstancePeer\n");

    ASSERT(m_pINS4AdapterPeer != NULL);

    if (m_pINS4AdapterPeer != NULL)
    {
	if (m_attribute_list != NULL && m_values_list != NULL) 
	{
	    for (int i = 0; i < m_attribute_cnt; i++)   
	    {
		m_pINS4AdapterPeer->NPN_MemFree(m_attribute_list[i]);
		m_pINS4AdapterPeer->NPN_MemFree(m_values_list[i]);
	    }

	    m_pINS4AdapterPeer->NPN_MemFree(m_attribute_list);
	    m_pINS4AdapterPeer->NPN_MemFree(m_values_list);
	}

	m_pINS4AdapterPeer->Release();
	m_pINS4AdapterPeer = NULL;
    }
}   

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInstancePeer::GetMIMEType
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginInstancePeer::GetMIMEType(JDPluginMimeType *result) 
{
    TRACE("CNS4Adapter_PluginInstancePeer::GetMIMEType\n");

    ASSERT(result != NULL);

    *result = m_typeString;
    return JD_OK;
}

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInstancePeer::GetAttributes
//=--------------------------------------------------------------------------=
// Get a ptr to the paired list of attribute names and values,
// returns the length of the array.
//
// Each name or value is a null-terminated string.
//
JD_METHOD
CNS4Adapter_PluginInstancePeer::GetAttributes(JDUint16& n, const char* const*& names, const char* const*& values)  
{
    TRACE("CNS4Adapter_PluginInstancePeer::GetAttributes\n");

    n = m_attribute_cnt;
    names = m_attribute_list;
    values = m_values_list;

    return JD_OK;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInstancePeer::GetAttributes
//=--------------------------------------------------------------------------=
// Get the value for the named attribute.  Returns null
// if the attribute was not set.
//
JD_METHOD
CNS4Adapter_PluginInstancePeer::GetAttribute(const char* name, const char* *result) 
{
    TRACE("CNS4Adapter_PluginInstancePeer::GetAttribute\n");

    for (int i=0; i < m_attribute_cnt; i++)  
    {
        if (JDStrcasecmp(name, m_attribute_list[i]) == 0) 
        {
            *result = m_values_list[i];
            return JD_OK;
        }
    }

    return JD_ERROR_FAILURE;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInstancePeer::Version
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginInstancePeer::Version(int* plugin_major, int* plugin_minor,
	                                int* netscape_major, int* netscape_minor)
{
    TRACE("CNS4Adapter_PluginInstancePeer::Version\n");

    ASSERT(m_pINS4AdapterPeer != NULL);
    ASSERT(plugin_major != NULL);
    ASSERT(plugin_minor != NULL);
    ASSERT(netscape_major != NULL);
    ASSERT(netscape_minor != NULL);

    if (m_pINS4AdapterPeer != NULL)
	m_pINS4AdapterPeer->NPN_Version(plugin_major, plugin_minor, 
					     netscape_major, netscape_minor);

    return JD_OK;
}


JD_METHOD
CNS4Adapter_PluginInstancePeer::GetJSThread(JDUint32 *outThreadID)
{
    return JD_ERROR_NOT_IMPLEMENTED;
}

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInstancePeer::ShowStatus
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginInstancePeer::ShowStatus(const char* message)
{
    TRACE("CNS4Adapter_PluginInstancePeer::ShowStatus\n");

    ASSERT(m_pINS4AdapterPeer != NULL);
    ASSERT(message != NULL);

    if (m_pINS4AdapterPeer != NULL)
	m_pINS4AdapterPeer->NPN_Status(m_npp, message);

    return JD_OK;
}



//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInstancePeer::SetWindowSize
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginInstancePeer::SetWindowSize(JDUint32 width, JDUint32 height)
{
    return JD_ERROR_NOT_IMPLEMENTED;
}


