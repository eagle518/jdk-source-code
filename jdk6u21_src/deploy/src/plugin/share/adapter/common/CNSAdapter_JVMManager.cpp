/*
 * @(#)CNSAdapter_JVMManager.cpp	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_JVMManager.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_JVMManager.cpp: Implementation of adapter for nsIJVMManager
//

#include "StdAfx.h"
#include "jni.h"
#include "IJVMManager.h"
#include "IThreadManager.h"
#include "ISecureEnv.h"
#include "nsIJVMManager.h"
#include "nsIThreadManager.h"
#include "CNSAdapter_SecureJNIEnv.h"
#include "CNSAdapter_JVMManager.h"

#include "Debug.h"

// Netscape iid
static NS_DEFINE_IID(kIJVMManagerIID, NS_IJVMMANAGER_IID);
static NS_DEFINE_IID(kIThreadManagerIID, NS_ITHREADMANAGER_IID);

//ISupports
JD_IMPL_ISUPPORTS2(CNSAdapter_JVMManager, IJVMManager, IThreadManager);

//=--------------------------------------------------------------------------=
// CNSAdapter_JVMManager::CNSAdapter_JVMManager
//=--------------------------------------------------------------------------=
//
// notes :
//	
CNSAdapter_JVMManager::CNSAdapter_JVMManager(nsIJVMManager* pJVMManager) :
    m_pJVMManager(pJVMManager), m_pThreadManager(NULL)
{
    TRACE("CNSAdapter_JVMManager::CNSAdapter_JVMManager\n");

    JD_INIT_REFCNT();
    
    if (m_pJVMManager != NULL)
    {
	m_pJVMManager->AddRef();
	m_pJVMManager->QueryInterface(kIThreadManagerIID, (void**)&m_pThreadManager);
    }
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JVMManager::~CNSAdapter_JVMManager
//=--------------------------------------------------------------------------=
//
// notes :
//
CNSAdapter_JVMManager::~CNSAdapter_JVMManager()
{
    TRACE("CNSAdapter_JVMManager::~CNSAdapter_JVMManager\n");

    if (m_pJVMManager != NULL)
	m_pJVMManager->Release();

    if (m_pThreadManager != NULL)
	m_pThreadManager->Release();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JVMManager::CreateProxyJNI
//=--------------------------------------------------------------------------=
// param: JNIEnv* *outProxyEnv contains the result of Proxy JNIEnv
//
// notes :
//   This call first gets nsIJVMManager and then call CreateProxyJNI with
//   "NULL" for the first argument, this will result into calling into 
//   nsIJVMPlugin(JavaPluginFactory) to call CreateSecureEnv to have nsISecureEnv
//   wrapped into ProxyJNI object
//
JD_METHOD
CNSAdapter_JVMManager::CreateProxyJNI(ISecureEnv* secureEnv, JNIEnv* *outProxyEnv)
{
    TRACE("CNSAdapter_JVMManager::CreateProxyJNI\n");

    if (outProxyEnv == NULL || m_pJVMManager == NULL)
	return JD_ERROR_NULL_POINTER;

    *outProxyEnv = NULL;
    
    JDSmartPtr<nsISecureEnv> spSecureEnv = NULL;
    if (secureEnv != NULL)
        spSecureEnv = new CNSAdapter_SecureJNIEnv(secureEnv);
    
    return m_pJVMManager->CreateProxyJNI(spSecureEnv, outProxyEnv);

}

//=--------------------------------------------------------------------------=
// CNSAdapter_JVMManager::IsAllPermissionGranted
//=--------------------------------------------------------------------------=
// This method is used when doing RSA verification. it has been removed already
// notes :
//
JD_METHOD
CNSAdapter_JVMManager::IsAllPermissionGranted(const char *lastFingerprint, 
					       const char *lastCommonName,
				               const char *rootFingerprint,
				               const char *rootCommonName, 
				               JDBool *_retval)
{
    if (m_pJVMManager == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pJVMManager->IsAllPermissionGranted(lastFingerprint,
						 lastCommonName,
						 rootFingerprint,
						 rootCommonName,
    					         _retval);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JVMManager::GetCurrentThread
//=--------------------------------------------------------------------------=
// params:  JDUint32 *threadID contains the thread ID result
// return:  JD_OK if call succeed
// notes :
//
JD_METHOD
CNSAdapter_JVMManager::GetCurrentThread(JDUint32 *threadID)
{
    TRACE("CNSAdapter_JVMManager::GetCurrentThread\n");
    
    if (m_pThreadManager == NULL)
	return JD_ERROR_NULL_POINTER;

    nsresult res = JD_OK;
    
    PRThread * pPluginThread = NULL;
    res = m_pThreadManager->GetCurrentThread(&pPluginThread);

    if (NS_SUCCEEDED(res) && pPluginThread)
	*threadID = (JDUint32)pPluginThread;

    return res;
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JVMManager::PostEvent
//=--------------------------------------------------------------------------=
// params:    threadID:  the thread id which the event will be posted
//	      runnable:  the call back object			
//	       async:	 TRUE if asynchrous
// return:    JD_OK of call succeed
// notes :
//	
JD_METHOD
CNSAdapter_JVMManager::PostEvent(JDUint32 threadID, IRunnable* runnable, JDBool async)
{
    TRACE("CNSAdapter_JVMManager::PostEvent\n");
    if (m_pThreadManager == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsIRunnable> spRunnable = new CNSAdapter_JSCallDispatcher(runnable);

    if (spRunnable == NULL)
	return JD_ERROR_OUT_OF_MEMORY;

    return m_pThreadManager->PostEvent((PRThread *)threadID, spRunnable, async);
}

//nsISupports
NS_IMPL_ISUPPORTS1(CNSAdapter_JSCallDispatcher, nsIRunnable);

//=--------------------------------------------------------------------------=
// CNSAdapter_JSCallDispatcher::CNSAdapter_JSCallDispatcher
//=--------------------------------------------------------------------------=
//
// notes :
//
CNSAdapter_JSCallDispatcher::CNSAdapter_JSCallDispatcher(IRunnable* pRunnable) : 
m_pRunnable(pRunnable){
    
    NS_INIT_REFCNT();
    if (m_pRunnable)
	m_pRunnable->AddRef();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JSCallDispatcher::~CNSAdapter_JSCallDispatcher
//=--------------------------------------------------------------------------=
//
// notes :
//
CNSAdapter_JSCallDispatcher::~CNSAdapter_JSCallDispatcher()
{
    if (m_pRunnable)
	m_pRunnable->Release();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_JSCallDispatcher::Run
//=--------------------------------------------------------------------------=
// Called when the event get the chance to be executed
// notes :
//
NS_METHOD 
CNSAdapter_JSCallDispatcher::Run() 
{ 
    if (m_pRunnable == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pRunnable->Run();
}
