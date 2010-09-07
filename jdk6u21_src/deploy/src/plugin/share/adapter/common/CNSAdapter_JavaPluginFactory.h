/*
 * @(#)CNSAdapter_JavaPluginFactory.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_JavaPluginFactory.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_JavaPluginFactory.h : Declaration of the Adapter for nsIPlugin.
// This is a service entry point JPI provided to browser
//
#ifndef _CNSAdapter_JavaPluginFactory_h__
#define _CNSAdapter_JavaPluginFactory_h__

#include "jni.h"
#include "nsIJVMPlugin.h"
#include "nsIPlugin.h"
#include "nsIJVMConsole.h"

class IJVMPlugin;
class IPlugin;
class IJVMConsole;
class IFactory;
/**
 *
 *          +------------+             +-----------+		 +----------+
 *          |  Netscape  |  <------->  |  Adapter  | <---------> |  Plug-in |
 *          +------------+	       +-----------+		 +----------+
 *             
 *  CJavaPluginFactory is an service entry point, it exposes the functionality such as
 *  create plugin instance, create java console, create secure JNI env etc
 *  The CNSAdapter_JavaPluginFactory is an adpater code which forwards the request from 
 *  the browser(Mozilla/NS6) to CJavaPluginFactory implementation.
 */
class CNSAdapter_JavaPluginFactory : public nsIJVMPlugin,
				     public nsIPlugin,
				     public nsIJVMConsole
{
public:
    //=--------------------------------------------------------------=
    // nsISupport
    //=--------------------------------------------------------------=
    NS_DECL_ISUPPORTS

    //=--------------------------------------------------------------=
    // nsIFactory
    //=--------------------------------------------------------------=
    NS_IMETHOD CreateInstance(nsISupports *outer, REFNSIID iid, void **result);

    NS_IMETHOD LockFactory(PRBool aLock);

    //=--------------------------------------------------------------=
    // nsIPlugin
    //=--------------------------------------------------------------=

    /**
     * Creates a new plugin instance, based on a MIME type. This
     * allows different impelementations to be created depending on
     * the specified MIME type.
     */
    NS_IMETHOD CreatePluginInstance(nsISupports *aOuter, REFNSIID aIID,
                                    const char* aPluginMIMEType,
                                    void **aResult);

    // (Corresponds to NPP_Initialize.)
    NS_IMETHOD Initialize(void);

    // (Corresponds to NPP_Shutdown.)
    NS_IMETHOD Shutdown(void);

    // (Corresponds to NPP_GetMIMEDescription.)
    NS_IMETHOD GetMIMEDescription(const char* *resultingDesc);

    // (Corresponds to NPP_GetValue.)
    NS_IMETHOD GetValue(nsPluginVariable variable, void *value);

    //=--------------------------------------------------------------=
    // nsIJVMPlugin
    //=--------------------------------------------------------------=

    // Causes the JVM to append a new directory to its classpath.
    // If the JVM doesn't support this operation, an error is returned.
    NS_IMETHOD AddToClassPath(const char* dirPath);

    // Causes the JVM to remove a directory from its classpath.
    // If the JVM doesn't support this operation, an error is returned.
    NS_IMETHOD RemoveFromClassPath(const char* dirPath);

    // Returns the current classpath in use by the JVM.
    NS_IMETHOD GetClassPath(const char* *result);

    /**
     * This creates a new secure communication channel with Java. The second parameter,
     * nativeEnv, if non-NULL, will be the actual thread for Java communication.
     * Otherwise, a new thread should be created.
     * @param	proxyEnv		the env to be used by all clients on the browser side
     * @return	outSecureEnv	the secure environment used by the proxyEnv
     */
    NS_IMETHOD
    CreateSecureEnv(JNIEnv* proxyEnv, nsISecureEnv* *outSecureEnv);

    /**
     * Gives time to the JVM from the main event loop of the browser. This is
     * necessary when there aren't any plugin instances around, but Java threads exist.
     */
    NS_IMETHOD
    SpendTime(PRUint32 timeMillis);

    //=--------------------------------------------------------------=
    // Create JSObject wrapper
    //=--------------------------------------------------------------=
    NS_IMETHOD GetJavaWrapper(JNIEnv* jenv, jint obj, jobject *jobj);

    NS_IMETHOD UnwrapJavaWrapper(JNIEnv* jenv, jobject jobj, jint* obj);

    // nsIJVMConsole
    NS_IMETHOD	Show(void);

    NS_IMETHOD  Hide(void);

    NS_IMETHOD  IsVisible(PRBool *result);

    NS_IMETHOD	Print(const char* msg, const char* encodingName = NULL);

    CNSAdapter_JavaPluginFactory(IFactory* p_JavaPluginFactory);

    virtual ~CNSAdapter_JavaPluginFactory(void);

protected:
    // Since CNSAdapter_JavaPluginFactory is used often, we did
    // some optimization to cache the three interface pointers rather
    // than QueryInterface everytime it is used.
    IJVMPlugin*	    m_pIJVMPlugin;
    IPlugin*	    m_pIPlugin;
    IJVMConsole*    m_pIJVMConsole;
};

#endif	//_CNSAdapter_JavaPluginFactory_h__
