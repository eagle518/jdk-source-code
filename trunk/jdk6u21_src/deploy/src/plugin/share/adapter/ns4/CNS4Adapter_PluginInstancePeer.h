/*
 * @(#)CNS4Adapter_PluginInstancePeer.h	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInstancePeer.h    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CNS4Adapter_PluginInstancePeer.h: interface for the 
//				    CNS4Adapter_PluginInstancePeer class.
//
// This is the dummy instance peer that interacts with the 5.0 plugin.
// In order to do LiveConnect, the class subclasses nsILiveConnectPluginInstancePeer.
//

#if !defined(AFX_CNS4Adapter_PluginInstancePeer_H__3651882B_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_)
#define AFX_CNS4Adapter_PluginInstancePeer_H__3651882B_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IPluginInstancePeer.h"
#include "IPluginTagInfo.h"

class INS4AdapterPeer;
class IInputStream;

class CNS4Adapter_PluginInstancePeer : public IPluginInstancePeer, 
				       public IPluginTagInfo 
{
public:

    // XXX - I add parameters to the constructor because I wasn't sure if
    // XXX - the 4.0 browser had the npp_instance struct implemented.
    // XXX - If so, then I can access npp_instance through npp->ndata.
    CNS4Adapter_PluginInstancePeer(INS4AdapterPeer* peer,
				   NPP npp, JDPluginMimeType typeString, 
				   JDUint16 attribute_cnt, 
				   const char** attribute_list, 
				   const char** values_list);

    virtual ~CNS4Adapter_PluginInstancePeer(void);

    JD_DECL_ISUPPORTS

    // Corresponds to NPP_New's MIMEType argument.
    JD_IMETHOD
    GetMIMEType(JDPluginMimeType *result);

    // Corresponds to NPN_ShowStatus.
    JD_IMETHOD
    ShowStatus(const char* message);

    /**
     * Set the desired size of the window in which the plugin instance lives.
     *
     * @param width - new window width
     * @param height - new window height
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    SetWindowSize(JDUint32 width, JDUint32 height);
    
    JD_IMETHOD
    GetJSThread(JDUint32 *outThreadID);
    
    // Get a ptr to the paired list of attribute names and values,
    // returns the length of the array.
    //
    // Each name or value is a null-terminated string.
    JD_IMETHOD
    GetAttributes(JDUint16& n, const char* const*& names, const char* const*& values);

    // Get the value for the named attribute.  Returns null
    // if the attribute was not set.
    JD_IMETHOD
    GetAttribute(const char* name, const char* *result);

    
    // XXX - Where did this go?
    JD_IMETHOD
    Version(int* plugin_major, int* plugin_minor,
            int* netscape_major, int* netscape_minor);

    NPP GetNPPInstance(void)   {
	return m_npp;
    }

protected:

    INS4AdapterPeer*     m_pINS4AdapterPeer;

    NPP m_npp;

    // XXX - The next five variables may need to be here since I
    // XXX - don't think np_instance is available in 4.0X.
    JDPluginMimeType	 m_typeString;
    JDUint16		 m_attribute_cnt;
    char**		 m_attribute_list;
    char**		 m_values_list;
};

#endif // !defined(AFX_CBAPLUGININSTANCEPEER_H__3651882B_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_)
