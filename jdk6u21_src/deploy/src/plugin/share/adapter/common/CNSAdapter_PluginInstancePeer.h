/*
 * @(#)CNSAdapter_PluginInstancePeer.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_PluginInstancePeer.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_PluginInstancePeer.h is Declaration of adapter for nsIPluginInstancePeer
// 
//
#ifndef _CNSAdapter_PluginInstancePeer_h__
#define _CNSAdapter_PluginInstancePeer_h__

#include "IPluginInstancePeer.h"
#include "IPluginTagInfo2.h"

class nsIPluginInstancePeer;
class nsIPluginTagInfo2;

class CNSAdapter_PluginInstancePeer : public IPluginInstancePeer,
				      public IPluginTagInfo2
{
public:
    //ISupports
    JD_DECL_ISUPPORTS

    // IPluginInstancePeer
    JD_IMETHOD
    GetMIMEType(JDPluginMimeType *result);
    
    JD_IMETHOD
    ShowStatus(const char* message);

    JD_IMETHOD
    SetWindowSize(JDUint32 width, JDUint32 height);

    JD_IMETHOD
    GetJSThread(JDUint32 *outThreadID);

    // IPluginInfo
    JD_IMETHOD
    GetAttributes(JDUint16& n, const char*const*& names, const char*const*& values);

    JD_IMETHOD
    GetAttribute(const char* name, const char* *result);

    //IPluginTagInfo2
    JD_IMETHOD
    GetTagType(JDPluginTagType *result);

    // Get the complete text of the HTML tag that was
    // used to instantiate this plugin
    JD_IMETHOD
    GetTagText(const char* *result);

    // Get a ptr to the paired list of parameter names and values,
    // returns the length of the array.
    //
    // Each name or value is a null-terminated string.
    JD_IMETHOD
    GetParameters(PRUint16& n, const char*const*& names, const char*const*& values);

    // Get the value for the named parameter.  Returns null
    // if the parameter was not set.
    // @result - NS_OK if this operation was successful, NS_ERROR_FAILURE if
    // this operation failed. result is set to NULL if the attribute is not found
    // else to the found value.
    JD_IMETHOD
    GetParameter(const char* name, const char* *result);

    JD_IMETHOD
    GetDocumentBase(const char* *result);

    // Return an encoding whose name is specified in:
    // http://java.sun.com/products/jdk/1.1/docs/guide/intl/intl.doc.html#25303
    JD_IMETHOD
    GetDocumentEncoding(const char* *result);

    JD_IMETHOD
    GetWidth(JDUint32 *result);

    JD_IMETHOD
    GetHeight(JDUint32 *result);

    // Returns a unique id for the current document on which the
    // plugin is displayed.
    JD_IMETHOD
    GetUniqueID(JDUint32 *result);

    // Constructor
    CNSAdapter_PluginInstancePeer(nsIPluginInstancePeer* pPluginInstancePeer);

    virtual ~CNSAdapter_PluginInstancePeer();
private:
    nsIPluginInstancePeer* m_pPluginInstancePeer;
    nsIPluginTagInfo2*     m_pPluginTagInfo2;
};

#endif //_CNSAdapter_PluginInstancePeer_h__
	
