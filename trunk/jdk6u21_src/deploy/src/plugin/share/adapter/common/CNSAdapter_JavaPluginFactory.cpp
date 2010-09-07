/*
 * @(#)CNSAdapter_JavaPluginFactory.cpp	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_JavaPluginFactory.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_JavaPluginFactory.cpp: Implementation of adapter for nsIPlugin,
// See CNSAdapter_JavaPluginFactory.h for details.
//
#include "StdAfx.h"
#include "jni.h"
#include "nsAgg.h"
#include "nsIServiceManager.h"
#include "nsIPluginManager.h"
#include "nsIJVMManager.h"
#include "nsIJVMConsole.h"
#include "nsILiveconnect.h"
#include "nsISecureEnv.h"
#include "nsIJVMPlugin.h"
#include "nsIPlugin.h"
#include "IPlugin.h"
#include "IJVMPlugin.h"
#include "IJVMConsole.h"
#include "ISecureEnv.h"
#include "CNSAdapter_SecureJNIEnv.h"
#include "CNSAdapter_JavaPluginFactory.h"

#ifdef XP_WIN
#include "clientLoadCOM.h"
#endif 
#include "Debug.h"

// Netscape iid
static NS_DEFINE_IID(kILiveConnectIID, NS_ILIVECONNECT_IID);
static NS_DEFINE_IID(kIJVMPluginIID, NS_IJVMPLUGIN_IID);
static NS_DEFINE_IID(kIPluginIID, NS_IPLUGIN_IID);
static NS_DEFINE_IID(kIFactoryIID, NS_IFACTORY_IID);
static NS_DEFINE_IID(kIPluginManagerIID, NS_IPLUGINMANAGER_IID);
static NS_DEFINE_IID(kIJVMConsoleIID, NS_IJVMCONSOLE_IID);
static NS_DEFINE_CID(kCPluginManagerCID, NS_PLUGINMANAGER_CID);
static NS_DEFINE_CID(kCLiveConnectCID, NS_CLIVECONNECT_CID);
static NS_DEFINE_CID(kCJVMManagerCID, NS_JVMMANAGER_CID);

// JPI iid
static JD_DEFINE_IID(jIJVMPluginIID, IJVMPLUGIN_IID);
static JD_DEFINE_IID(jIPluginIID, IPLUGIN_IID);
static JD_DEFINE_IID(jIJVMConsoleIID, IJVMCONSOLE_IID);

// nsISupports implementation
NS_IMPL_ISUPPORTS3(CNSAdapter_JavaPluginFactory, nsIJVMPlugin, nsIPlugin, nsIJVMConsole);

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::CNSAdapter_JavaPluginFactory
//=---------------------------------------------------------------------------=
// Implements the CNSAdapter_JavaPluginFactory object for encapsulating the win32 plugin
// factory.
//
// parameters :
//
// return :
//
// notes :
//
CNSAdapter_JavaPluginFactory::CNSAdapter_JavaPluginFactory(IFactory* p_JavaPluginFactory)
: m_pIJVMPlugin(NULL), m_pIPlugin(NULL), m_pIJVMConsole(NULL)
{
    TRACE("CNSAdapter_JavaPluginFactory::CNSAdapter_JavaPluginFactory");
    NS_INIT_REFCNT();

    if (p_JavaPluginFactory != NULL)
    {
	// Get the three interface pointers
	p_JavaPluginFactory->QueryInterface(jIJVMPluginIID, (void**)&m_pIJVMPlugin);
	p_JavaPluginFactory->QueryInterface(jIPluginIID, (void**)&m_pIPlugin);
	p_JavaPluginFactory->QueryInterface(jIJVMConsoleIID, (void**)&m_pIJVMConsole);
    }
}



///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::~CNSAdapter_JavaPluginFactory
//=---------------------------------------------------------------------------=
// 
// parameters :
//
// return :
//
// notes :
//
CNSAdapter_JavaPluginFactory::~CNSAdapter_JavaPluginFactory()
{
    TRACE("CNSAdapter_JavaPluginFactory::~CNSAdapter_JavaPluginFactory");

    if (m_pIJVMPlugin != NULL)
	m_pIJVMPlugin->Release();

    if (m_pIPlugin != NULL)
	m_pIPlugin->Release();

    if (m_pIJVMConsole != NULL)
	m_pIJVMConsole->Release();
}

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::Initialize
//=---------------------------------------------------------------------------=
// Initialize is called when the plugin DLL is loaded and initialized.
//
// parameters :
//	nsISupports		 [in]	 Interface to plugin manager
//
// return :
//	nsresult	     [out]   Success code
//
// notes :
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::Initialize(void)
{
    TRACE("CNSAdapter_JavaPluginFactory::Initialize\n");
    
    if (m_pIPlugin == NULL)
	return NS_ERROR_NULL_POINTER;
    
    return m_pIPlugin->Initialize();
}


///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::Shutdown
//=---------------------------------------------------------------------------=
// Shutdown is called when the plugin DLL is shutdown and unloaded.
//
// parameters :
//
// return :
//	nsresult    [out]   Success code
//
// notes :
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::Shutdown(void)
{
    TRACE("CNSAdapter_JavaPluginFactory::Shutdown\n");
    
    if (m_pIPlugin == NULL)
	return NS_ERROR_NULL_POINTER;
    
    return m_pIPlugin->Shutdown();
}


///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::CreateInstance
//=---------------------------------------------------------------------------=
// CreateInstance is called when a new plugin instance needs to be created,
// based on a MIME type.
//
// parameters :
//		nsISupports* [in]    Outer object for aggregation
//		REFNSIID     [in]    IID of the interface requested
//		void**	     [out]   Result
//
// return :
//		nsresult     [out]   Success code
//
// notes :
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::CreatePluginInstance(nsISupports *outer, REFNSIID iid,
							          const char* aPluginMIMEType,
							          void **result)
{
    TRACE("CNSAdapter_JavaPluginFactory::CreateJavaPluginInstance\n");
    
    return CreateInstance(outer, iid, result);
}

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::CreateInstance
//=---------------------------------------------------------------------------=
// CreateInstance is called when a new plugin instance needs to be created.
//
// parameters :
//		nsISupports* [in]    Outer object for aggregation
//		REFNSIID     [in]    IID of the interface requested
//		void**	     [out]   Result
//
// return :
//		nsresult     [out]   Success code
//
// notes :
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::CreateInstance(nsISupports *outer, REFNSIID iid, void **result)
{
    //This method must be implemented by different adapters
    TRACE("You should not be here\n");
    return NS_ERROR_NOT_IMPLEMENTED;
}

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::LockFactory
//=---------------------------------------------------------------------------=
// LockFactory locks plugin DLL in memory.
//
// parameters :
//		PRBool	[in]    TRUE if DLL should not be unloaded
//
// return :
//		nsresult    [out]   Success code
//
// notes :
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::LockFactory(PRBool aLock)
{
    TRACE("CNSAdapter_JavaPluginFactory::LockFactory\n");
    if (m_pIPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pIPlugin->LockFactory(aLock);
}


///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::GetMIMEDescription
//=---------------------------------------------------------------------------=
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::GetMIMEDescription(const char* *resultingDesc)
{
    TRACE("CNSAdapter_JavaPluginFactory::GetMIMEDescription\n");
    if (m_pIPlugin == NULL)
	return NS_ERROR_NULL_POINTER;
    
    return m_pIPlugin->GetMIMEDescription(resultingDesc);
}


///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::GetValue
//=---------------------------------------------------------------------------=
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::GetValue(nsPluginVariable variable, void *value)
{
    TRACE("CNSAdapter_JavaPluginFactory::GetValue\n");
    
    JDPluginVariable jdVariable;

    switch (variable) {

    case nsPluginVariable_NameString:
      jdVariable = JDPluginVariable_NameString;
      break;

    case nsPluginVariable_DescriptionString:
      jdVariable = JDPluginVariable_DescriptionString;
      break;

    default:
      return NS_ERROR_FAILURE;
    }

    return m_pIPlugin->GetValue(jdVariable, value);
}

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::AddToClassPath
//=---------------------------------------------------------------------------=
//
// Causes the JVM to append a new directory to its classpath.
// If the JVM doesn't support this operation, an error is returned.
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::AddToClassPath(const char* dirPath)
{
    TRACE("CNSAdapter_JavaPluginFactory::AddToClassPath\n");
    
    return NS_ERROR_NOT_IMPLEMENTED;
}


///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::RemoveFromClassPath
//=---------------------------------------------------------------------------=
//
// Causes the JVM to remove a directory from its classpath.
// If the JVM doesn't support this operation, an error is returned.
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::RemoveFromClassPath(const char* dirPath)
{
    TRACE("CNSAdapter_JavaPluginFactory::RemoveFromClassPath\n");
  
    return NS_ERROR_NOT_IMPLEMENTED;
}


///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::GetClassPath
//=---------------------------------------------------------------------------=
//
// Returns the current classpath in use by the JVM.
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::GetClassPath(const char* *result)
{
    TRACE("CNSAdapter_JavaPluginFactory::GetClassPath\n");
    
   if (m_pIJVMPlugin == NULL)
	return NS_ERROR_NULL_POINTER;
    
    return m_pIJVMPlugin->GetClassPath(result);
}

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::CreateSecureEnv
//=---------------------------------------------------------------------------=
//
// Find or create a nsISecureEnv for the current thread.
// Returns NULL if an error occurs.
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::CreateSecureEnv(JNIEnv* proxyEnv, nsISecureEnv** result)
{

    TRACE("CNSAdapter_JavaPluginFactory::CreateSecureEnv\n");
    if (result == NULL || m_pIJVMPlugin == NULL)
	return NS_ERROR_NULL_POINTER;

    *result = NULL;

    JDSmartPtr<ISecureEnv> spOutSecureEnv;
    
    JDresult err = m_pIJVMPlugin->CreateSecureEnv(proxyEnv, &spOutSecureEnv);
    if (JD_SUCCEEDED(err) && spOutSecureEnv != NULL)
    {
	*result = new CNSAdapter_SecureJNIEnv(spOutSecureEnv);

	if (*result == NULL)
	    return NS_ERROR_OUT_OF_MEMORY;
	// AddRef the outer pointer
	(*result)->AddRef();
    }

    return err;

}

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::SpendTime
//=---------------------------------------------------------------------------=
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::SpendTime(PRUint32 timeMillis)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}


///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::GetJavaWrapper
//=---------------------------------------------------------------------------=
//
// Create Java JSObject wrapper.
// param:  jint   Native JS object hash value
//	   jobj   the returned Java object 
//
// notes:
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::GetJavaWrapper(JNIEnv* jenv, jint obj, jobject *jobj)
{
    TRACE("CNSAdapter_JavaPluginFactory::GetJavaWrapper\n");

    if (m_pIJVMPlugin == NULL)
	return NS_ERROR_NULL_POINTER;
    
    return m_pIJVMPlugin->GetJavaWrapper(jenv, obj, jobj);
}


///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::UnwrapJavaWrapper
//=---------------------------------------------------------------------------=
//
// Create Java JSObject wrapper.
//   
// params:   jobj Java object
//           obj returned Native JS object hash value
//
// notes:
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::UnwrapJavaWrapper(JNIEnv* jenv, jobject jobj, jint* obj)
{
    TRACE("CNSAdapter_JavaPluginFactory::UnwrapJavaWrapper\n");
    if (m_pIJVMPlugin == NULL)
	return NS_ERROR_NULL_POINTER;
    
    return m_pIJVMPlugin->UnwrapJavaWrapper(jenv, jobj, obj);
}

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::Show
//=---------------------------------------------------------------------------=
//
// Show Java Console
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::Show(void)
{
    TRACE("CNSAdapter_JavaPluginFactory::Show\n");
    if (m_pIJVMConsole == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pIJVMConsole->Show();
}

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::Hide
//=---------------------------------------------------------------------------=
//
// Hide Java Console
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::Hide(void)
{
    TRACE("CNSAdapter_JavaPluginFactory::Hide\n");
     if (m_pIJVMConsole == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pIJVMConsole->Hide();
}

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::IsVisible
//=---------------------------------------------------------------------------=
//  
// params:  result TRUE if it is visible
//
// notes:
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::IsVisible(PRBool *result)
{
    TRACE("CNSAdapter_JavaPluginFactory::IsVisible\n");
     if (m_pIJVMConsole == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pIJVMConsole->IsVisible(result);
}

///=--------------------------------------------------------------------------=
// CNSAdapter_JavaPluginFactory::Print
//=---------------------------------------------------------------------------=
//
//
//
NS_IMETHODIMP CNSAdapter_JavaPluginFactory::Print(const char* msg, const char* encodingName)
{
    TRACE("CNSAdapter_JavaPluginFactory::Print\n");
     if (m_pIJVMConsole == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pIJVMConsole->Print(msg, encodingName);
}
