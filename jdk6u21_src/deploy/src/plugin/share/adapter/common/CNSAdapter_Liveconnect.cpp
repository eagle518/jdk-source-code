/*
 * @(#)CNSAdapter_Liveconnect.cpp	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_Liveconnect.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_Liveconnect.cpp: Implementation of adapter for nsILiveconnect
//
#include "StdAfx.h"
#include "jni.h"
#include "ILiveconnect.h"
#include "ISecurityContext.h"
#include "nsILiveconnect.h"
#include "nsISecurityContext.h"
#include "CNSAdapter_Liveconnect.h"
#include "CNSAdapter_SecurityContext.h"
#include "CNSAdapter_JavaPlugin.h"
#include "CMap.h"

#include "Debug.h"

// This map is to get the corresponding CNSAdapter_JavaPlugin object associated with CJavaPlugin object.
// Reason: Plug-in only knows CJavaPlugin, so in the adapter, we maintain a 1x1 mapping between CJavaPluginAdapter
// and CJavaPlugin
extern CMap<void*, void*> pluginMap;

static JD_DEFINE_IID(jISecurityContextIID, ISECURITYCONTEXT_IID);

// ISupports
JD_IMPL_ISUPPORTS1(CNSAdapter_Liveconnect, ILiveconnect);

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::CNSAdapter_Liveconnect
//=--------------------------------------------------------------------------=
//
// notes :
//	
CNSAdapter_Liveconnect::CNSAdapter_Liveconnect(nsILiveconnect* pLiveconnect) : 
    m_pLiveconnect(pLiveconnect) 
{
    TRACE("CNSAdapter_Liveconnect::CNSAdapter_Liveconnect\n");
    JD_INIT_REFCNT();

    if (m_pLiveconnect)
	m_pLiveconnect->AddRef();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::CNSAdapter_Liveconnect
//=--------------------------------------------------------------------------=
//
// notes :
// 
CNSAdapter_Liveconnect::~CNSAdapter_Liveconnect() 
{
    if (m_pLiveconnect)
	m_pLiveconnect->Release();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::CreateSecurityContext
//=--------------------------------------------------------------------------=
// params: securitySupports: SecurityContext 
//	   securityContext: the CSecurityContextAdapter 
// return: JD_OK if call succeed
// notes :
// 
JDresult 
CNSAdapter_Liveconnect::CreateSecurityContext(ISupports *securitySupports, nsISupports* *securityContext)
{
    if (securityContext == NULL)
	return JD_ERROR_NULL_POINTER;

    JDresult res;
    JDSmartPtr<ISecurityContext> spSecurityContext;

    res = securitySupports->QueryInterface(jISecurityContextIID, (void**)&spSecurityContext);

    if (JD_SUCCEEDED(res) && spSecurityContext)
    {
	*securityContext = new CNSAdapter_SecurityContext(spSecurityContext);
	if (*securityContext == NULL)
	    return JD_ERROR_OUT_OF_MEMORY;

	(*securityContext)->AddRef();
    }

    return res;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::GetMember
//=--------------------------------------------------------------------------=
//
// params: jsObj: the native JS object
//	   name:  the name of the member
//	   pjobj:  the result native JS object
// return: JD_OK if call succeed
// notes :
//
JD_METHOD
CNSAdapter_Liveconnect::GetMember(JNIEnv *jEnv, jsobject jsobj, const jchar *name, jsize length, void* principalsArray[], 
			           int numPrincipals, ISupports *securitySupports, jobject *pjobj) 
{
    if (m_pLiveconnect == NULL)
	return JD_ERROR_NULL_POINTER;

    // Convert the ISecurityContext to nsISecurityContext 
    JDSmartPtr<nsISupports> spSecurityContext;
    JDresult err = CreateSecurityContext(securitySupports, &spSecurityContext);
    
    if (JD_SUCCEEDED(err))
	err = m_pLiveconnect->GetMember(jEnv, jsobj, name, length, principalsArray,
					numPrincipals, spSecurityContext, pjobj);
    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::GetSlot
//=--------------------------------------------------------------------------=
// params:  jsobj: the native JS object
//	    slot: the index 
//	    pjobj: the result native js object
// return:  JD_OK if call succeed
// notes :
//
JD_METHOD
CNSAdapter_Liveconnect::GetSlot(JNIEnv *jEnv, jsobject jsobj, jint slot, void* principalsArray[], 
			         int numPrincipals, ISupports *securitySupports, jobject *pjobj)
{
    if (m_pLiveconnect == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsISupports> spSecurityContext;
    JDresult err = CreateSecurityContext(securitySupports, &spSecurityContext);
    
    if (JD_SUCCEEDED(err))
	err = m_pLiveconnect->GetSlot(jEnv, jsobj, slot, principalsArray,
				      numPrincipals, spSecurityContext, pjobj);

    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::SetMember
//=--------------------------------------------------------------------------=
// params:  jsobj: the native JS Object
//	    name:  the name of the member
//	    jobj:  the value to set
// return:  JD_OK if call succeed
// notes :
//
JD_METHOD
CNSAdapter_Liveconnect::SetMember(JNIEnv *jEnv, jsobject jsobj, const jchar* name, jsize length, jobject jobj, void* principalsArray[], 
				   int numPrincipals, ISupports *securitySupports)
{
    if (m_pLiveconnect == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsISupports> spSecurityContext;
    JDresult err = CreateSecurityContext(securitySupports, &spSecurityContext);
    
    if (JD_SUCCEEDED(err))
	err = m_pLiveconnect->SetMember(jEnv, jsobj, name, length, jobj, principalsArray,
					numPrincipals, spSecurityContext);
    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::SetSlot
//=--------------------------------------------------------------------------=
// params:  jsobj: the native JS object
//	    slot:  the index 
//	    jobj:  the object to set
// return:  JD_OK if call succeed
// notes :
//
JD_METHOD
CNSAdapter_Liveconnect::SetSlot(JNIEnv *jEnv, jsobject jsobj, jint slot, jobject jobj, void* principalsArray[], 
				 int numPrincipals, ISupports *securitySupports)
{
    if (m_pLiveconnect == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsISupports> spSecurityContext;
    JDresult err = CreateSecurityContext(securitySupports, &spSecurityContext);
    
    if (JD_SUCCEEDED(err))
	err = m_pLiveconnect->SetSlot(jEnv, jsobj, slot, jobj, principalsArray,
				      numPrincipals, spSecurityContext);
    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::RemoveMember
//=--------------------------------------------------------------------------=
// params:   jsobj: the native js object
//	     name: the name of the member
// return:   JD_Ok if call succeed
// notes :
//
JD_METHOD
CNSAdapter_Liveconnect::RemoveMember(JNIEnv *jEnv, jsobject jsobj, const jchar* name, jsize length,  void* principalsArray[], 
				      int numPrincipals, ISupports *securitySupports)
{
    if (m_pLiveconnect == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsISupports> spSecurityContext;
    JDresult err = CreateSecurityContext(securitySupports, &spSecurityContext);
    
    if (JD_SUCCEEDED(err))
	err = m_pLiveconnect->RemoveMember(jEnv, jsobj, name, length, principalsArray,
					   numPrincipals, spSecurityContext);

    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::Call
//=--------------------------------------------------------------------------=
// params: jsobj:   the native js object
//	   name:    the name of the function
//	   jobjArr: the list of params
//	   pjobj:   the return value
// return  JD_OK if call succeed 
// notes :
//
JD_METHOD
CNSAdapter_Liveconnect::Call(JNIEnv *jEnv, jsobject jsobj, const jchar* name, jsize length, jobjectArray jobjArr, void* principalsArray[], 
			      int numPrincipals, ISupports *securitySupports, jobject *pjobj)
{
    if (m_pLiveconnect == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsISupports> spSecurityContext;
    JDresult err = CreateSecurityContext(securitySupports, &spSecurityContext);
    
    if (JD_SUCCEEDED(err))
	err = m_pLiveconnect->Call(jEnv, jsobj, name, length, jobjArr, principalsArray,
				   numPrincipals, spSecurityContext, pjobj);
    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::CNSAdapter_Liveconnect
//=--------------------------------------------------------------------------=
// params: pJavaObject: the plugin object
//	    pobj:	the JS native object
// return: JD_OK if call succeed	
// notes :
//
JD_METHOD
CNSAdapter_Liveconnect::GetWindow(JNIEnv *jEnv, void *pJavaObject, void* principalsArray[], 
				   int numPrincipals, ISupports *securitySupports, jsobject *pobj)
{
    if (m_pLiveconnect == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsISupports> spSecurityContext;
    JDresult err = CreateSecurityContext(securitySupports, &spSecurityContext);
    
    if (JD_SUCCEEDED(err))
    {
	// Get CJavaPluginAdapter object from pluginMap
	CNSAdapter_JavaPlugin* pJavaObjectAdapter = (CNSAdapter_JavaPlugin*)pluginMap.FindElement(pJavaObject);

	err = m_pLiveconnect->GetWindow(jEnv, pJavaObjectAdapter, principalsArray,
					numPrincipals, spSecurityContext, pobj);
    }
    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::Eval
//=--------------------------------------------------------------------------=
// params:  obj:     the native JS object
//	    script:  the script
//	    pjobj:   the return value of evaluation
// return:  JD_OK    if call succeed 
// notes :
//
JD_METHOD	
CNSAdapter_Liveconnect::Eval(JNIEnv *jEnv, jsobject obj, const jchar *script, jsize length, void* principalsArray[], 
			      int numPrincipals, ISupports *securitySupports, jobject *pjobj)
{
    if (m_pLiveconnect == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsISupports> spSecurityContext;
    JDresult err = CreateSecurityContext(securitySupports, &spSecurityContext);
    
    if (JD_SUCCEEDED(err))
	err = m_pLiveconnect->Eval(jEnv, obj, script, length, principalsArray,
				   numPrincipals, spSecurityContext, pjobj);
    return err;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::FinalizeJSObject
//=--------------------------------------------------------------------------=
// params:  jsobj: the JS object to finalize
// return:  JD_OK if call succeed
// notes :
//
JD_METHOD
CNSAdapter_Liveconnect::FinalizeJSObject(JNIEnv *jEnv, jsobject jsobj)
{
    if (m_pLiveconnect == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pLiveconnect->FinalizeJSObject(jEnv, jsobj);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_Liveconnect::ToString
//=--------------------------------------------------------------------------=
// params: obj: a native JS object
//	   pjstring: the result string representation
// return  JD_OK if call succeed
// notes :
//
JD_METHOD
CNSAdapter_Liveconnect::ToString(JNIEnv *jEnv, jsobject obj, jstring *pjstring)
{
    if (m_pLiveconnect == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pLiveconnect->ToString(jEnv, obj, pjstring);
}
