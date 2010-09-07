/*
 * @(#)CNSAdapter_PluginInstancePeer.cpp	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_PluginInstancePeer.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_PluginInstancePeer.cpp is Implementation of adapter for 
// nsIPluginInstancePeer
// 
//
#include "StdAfx.h"
#include "IPluginInstancePeer.h"
#include "IPluginTagInfo2.h"
#include "nsIPluginInstancePeer.h"
#include "nsIPluginTagInfo2.h"
#include "nsIPluginTagInfo.h"
#include "nsIPluginInstancePeer2.h"
#include "CNSAdapter_PluginInstancePeer.h"

// Netscape iid
static NS_DEFINE_IID(kIPluginTagInfoIID, NS_IPLUGINTAGINFO_IID);
static NS_DEFINE_IID(kIPluginTagInfo2IID, NS_IPLUGINTAGINFO2_IID);
static NS_DEFINE_IID(kIPluginInstancePeer2IID, NS_IPLUGININSTANCEPEER2_IID);

// JPI iid
static JD_DEFINE_IID(jIPluginTagInfoIID, IPLUGINTAGINFO_IID);
static JD_DEFINE_IID(jIPluginTagInfo2IID, IPLUGINTAGINFO2_IID);
static JD_DEFINE_IID(jIPluginInstancePeerIID, IPLUGININSTANCEPEER_IID);
static JD_DEFINE_IID(jISupportsIID, ISUPPORTS_IID);

// ISupports
JD_IMPL_ADDREF(CNSAdapter_PluginInstancePeer);
JD_IMPL_RELEASE(CNSAdapter_PluginInstancePeer); 

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::QueryInterface
//=--------------------------------------------------------------------------=
// params: aIID: interface id
//	    result: the resulting interface reference
// return JD_OK if call succeed
// 
// notes :
//
JD_METHOD
CNSAdapter_PluginInstancePeer::QueryInterface(const JDIID& aIID, void** result)
{
    if (result == NULL)
	return JD_ERROR_NULL_POINTER;

    if (aIID.Equals(jIPluginInstancePeerIID))
	*result = (IPluginInstancePeer*)this;
    else if(aIID.Equals(jIPluginTagInfoIID))
	*result = (IPluginTagInfo*)this;
    else if (aIID.Equals(jIPluginTagInfo2IID) || aIID.Equals(jISupportsIID))
	*result = (IPluginTagInfo2*)this;
    else
	return JD_NOINTERFACE;

    AddRef();
    return JD_OK;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::CNSAdapter_PluginInstancePeer
//=--------------------------------------------------------------------------=
// params:  
//
// notes:
//
CNSAdapter_PluginInstancePeer::CNSAdapter_PluginInstancePeer(nsIPluginInstancePeer* pPluginInstancePeer)
: m_pPluginInstancePeer(pPluginInstancePeer)
{
    JD_INIT_REFCNT();

    if (m_pPluginInstancePeer != NULL)
    {
	m_pPluginInstancePeer->AddRef();
	m_pPluginInstancePeer->QueryInterface(kIPluginTagInfo2IID, (void**)&m_pPluginTagInfo2);
    }
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::~CNSAdapter_PluginInstancePeer
//=--------------------------------------------------------------------------=
// params:  
//
// notes:
//
CNSAdapter_PluginInstancePeer::~CNSAdapter_PluginInstancePeer()
{
    if (m_pPluginInstancePeer != NULL)
	m_pPluginInstancePeer->Release();

    if (m_pPluginTagInfo2 != NULL)
	m_pPluginTagInfo2->Release();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetMimeType
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
JD_METHOD
CNSAdapter_PluginInstancePeer::GetMIMEType(JDPluginMimeType *result)
{
    if (m_pPluginInstancePeer == NULL)
	return JD_ERROR_NULL_POINTER;
		
    return m_pPluginInstancePeer->GetMIMEType(result);
}


//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::ShowStatus
//=--------------------------------------------------------------------------=
// params: message the status message
// return: JD_OK if call succeed
// 
// notes :
//
JD_METHOD
CNSAdapter_PluginInstancePeer::ShowStatus(const char* message)
{
    if (m_pPluginInstancePeer == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pPluginInstancePeer->ShowStatus(message);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::SetWindowSize
//=--------------------------------------------------------------------------=
// params: width  the width to set
//	   height the height to set 
// 
// notes :
//
JD_METHOD
CNSAdapter_PluginInstancePeer::SetWindowSize(JDUint32 width, JDUint32 height)
{

    if (m_pPluginInstancePeer == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pPluginInstancePeer->SetWindowSize(width, height);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetJSThread
//=--------------------------------------------------------------------------=
// params: outThreadID the result thread id
// 
// notes :
//
JD_METHOD
CNSAdapter_PluginInstancePeer::GetJSThread(JDUint32 *outThreadID)
{
    if (m_pPluginInstancePeer == NULL)
	return JD_ERROR_NULL_POINTER;
    
    JDSmartPtr<nsIPluginInstancePeer2> spPluginInstancePeer2;
    nsresult err = m_pPluginInstancePeer->QueryInterface(kIPluginInstancePeer2IID, (void**)&spPluginInstancePeer2);
    
    if (NS_SUCCEEDED(err) && spPluginInstancePeer2)
	return spPluginInstancePeer2->GetJSThread(outThreadID);

    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetAttributes
//=--------------------------------------------------------------------------=
// params: n number of attributes
//	   names an array of names
//	   values the return attributes
// 
// notes :
//
JD_METHOD
CNSAdapter_PluginInstancePeer::GetAttributes(JDUint16& n, const char*const*& names, const char*const*& values)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;
	
    JDSmartPtr<nsIPluginTagInfo> spPluginTagInfo;
    nsresult err = m_pPluginTagInfo2->QueryInterface(kIPluginTagInfoIID, (void**)&spPluginTagInfo);
    
    if (NS_SUCCEEDED(err) && spPluginTagInfo)
	return spPluginTagInfo->GetAttributes(n, names, values);

    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetAttribute
//=--------------------------------------------------------------------------=
// params: name    name of the attribute
//	   result  the attribute value
// 
// notes :
//
JD_METHOD
CNSAdapter_PluginInstancePeer::GetAttribute(const char* name, const char* *result)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;
    
    JDSmartPtr<nsIPluginTagInfo> spPluginTagInfo;
    nsresult err = m_pPluginTagInfo2->QueryInterface(kIPluginTagInfoIID, (void**)&spPluginTagInfo);
    
    if (NS_SUCCEEDED(err) && spPluginTagInfo)
	err = spPluginTagInfo->GetAttribute(name, result);

    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetTagType
//=--------------------------------------------------------------------------=
// params: result  the Plugin type return value
// 
// notes :
//
JD_METHOD
CNSAdapter_PluginInstancePeer::GetTagType(JDPluginTagType *result)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;

    nsPluginTagType tagType = nsPluginTagType_Unknown;
       
  
    nsresult err = m_pPluginTagInfo2->GetTagType(&tagType);

    switch(tagType)
    {
    case nsPluginTagType_Embed:
	*result = JDPluginTagType_Embed;
	break;
    case nsPluginTagType_Object:
	*result = JDPluginTagType_Object;
	break;
    case nsPluginTagType_Applet:
	*result = JDPluginTagType_Applet;
	break;
    default:
	return JD_ERROR_FAILURE;
    }

    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetTagText
//=--------------------------------------------------------------------------=
// params: result  the text return value
// 
// notes :
//
// CNSAdapter_PluginInstancePeer::Get the complete text of the HTML tag that was
// used to instantiate this plugin
JD_METHOD
CNSAdapter_PluginInstancePeer::GetTagText(const char* *result)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;
   
    return m_pPluginTagInfo2->GetTagText(result);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetParameters
//=--------------------------------------------------------------------------=
// params:  n number of attributes
//	   names an array of names
//	   values the return Parameter
// 
// notes :
//
// CNSAdapter_PluginInstancePeer::Get a ptr to the paired list of parameter names and values,
// returns the length of the array.
//
// Each name or value is a null-terminated string.
JD_METHOD
CNSAdapter_PluginInstancePeer::GetParameters(PRUint16& n, const char*const*& names, const char*const*& values)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pPluginTagInfo2->GetParameters(n, names, values);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetParameter
//=--------------------------------------------------------------------------=
// params:  name   the name of parameter
//	    result the value of the parameter
//
// notes:
// CNSAdapter_PluginInstancePeer::Get the value for the named parameter.  Returns null
// if the parameter was not set.
// @result - JD_OK if this operation was successful, JD_ERROR_FAILURE if
// this operation failed. result is set to NULL if the attribute is not found
// else to the found value.
JD_METHOD
CNSAdapter_PluginInstancePeer::GetParameter(const char* name, const char* *result)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pPluginTagInfo2->GetParameter(name, result);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetDocumentBase
//=--------------------------------------------------------------------------=
// params:  result the returned document base
//
// notes:
//
JD_METHOD
CNSAdapter_PluginInstancePeer::GetDocumentBase(const char* *result)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pPluginTagInfo2->GetDocumentBase(result);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetDocumentEncoding
//=--------------------------------------------------------------------------=
// params:  result  the encoding type
//
// notes:
// Return an encoding whose name is specified in:
// http://java.sun.com/products/jdk/1.1/docs/guide/intl/intl.doc.html#25303
JD_METHOD
CNSAdapter_PluginInstancePeer::GetDocumentEncoding(const char* *result)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pPluginTagInfo2->GetDocumentEncoding(result);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetWidth
//=--------------------------------------------------------------------------=
// params:  result  width of the applet
//
// notes:
//
JD_METHOD
CNSAdapter_PluginInstancePeer::GetWidth(JDUint32 *result)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pPluginTagInfo2->GetWidth(result);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetHeight
//=--------------------------------------------------------------------------=
// params:  result  the height of the applet
//
// notes:
//
JD_METHOD
CNSAdapter_PluginInstancePeer::GetHeight(JDUint32 *result)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;
	
    return m_pPluginTagInfo2->GetHeight(result);
}


//=--------------------------------------------------------------------------=
// CNSAdapter_PluginInstancePeer::GetUniqueID
//=--------------------------------------------------------------------------=
// params:  result  the return id
//
// notes:
//
// Returns a unique id for the current document on which the
// plugin is displayed.
JD_METHOD
CNSAdapter_PluginInstancePeer::GetUniqueID(JDUint32 *result)
{
    if (m_pPluginTagInfo2 == NULL)
	return JD_ERROR_NULL_POINTER;
    
    return m_pPluginTagInfo2->GetUniqueID(result);
}

