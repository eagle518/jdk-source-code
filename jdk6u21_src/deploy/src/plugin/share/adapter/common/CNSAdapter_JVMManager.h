/*
 * @(#)CNSAdapter_JVMManager.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_JVMManager.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_JVMManager.h : Declaration of the Adapter for nsIJVMManager.
//

#ifndef __CNSAdapter_JVMManager_h_
#define __CNSAdapter_JVMManager_h_

#include "IJVMManager.h"
#include "IThreadManager.h"
#include "ISecureEnv.h"
#include "nsIThreadManager.h"

class IRunnable;
class nsIJVMManager;
// Java VM Plugin Manager is the additional entry points 
// Mozilla browser defines to support JVM plugins for browsers
// that support JVM plugin
class CNSAdapter_JVMManager : public IJVMManager,
			      public IThreadManager
{
public:
    // CNSAdapter_JVMManager methods
    CNSAdapter_JVMManager(nsIJVMManager* pJVMManager);
    virtual ~CNSAdapter_JVMManager();

    //ISUPPORTS
    JD_DECL_ISUPPORTS

    // IJVMManager
    // Creates a proxy JNI with an optional secure environment (which can be NULL).
    // There is a one-to-one correspondence between proxy JNIs and threads, so
    // calling this method multiple times from the same thread will return
    // the same proxy JNI.
    JD_IMETHOD
    CreateProxyJNI(ISecureEnv* secureEnv, JNIEnv* *outProxyEnv);

    // Used by plug-in to do RSA verification. Call has been removed from Plugin code
    JD_IMETHOD
    IsAllPermissionGranted(const char *lastFingerprint, 
			   const char *lastCommonName,
			   const char *rootFingerprint,
			   const char *rootCommonName, 
			   JDBool *_retval);

    //IThreadManager
    JD_IMETHOD
    GetCurrentThread(JDUint32 *threadID);
    
    JD_IMETHOD
    PostEvent(JDUint32 threadID, IRunnable* runnable, JDBool async);

private:
    nsIJVMManager*	m_pJVMManager;
    nsIThreadManager*	m_pThreadManager;
};

// CNSAdapter_JSCallDispatcher is used by PostEvent call
// the object contains the call back method  
class CNSAdapter_JSCallDispatcher: public nsIRunnable
{
public:
    //nsISupports
    NS_DECL_ISUPPORTS

    //CJSDispatcherAdapter methods
    CNSAdapter_JSCallDispatcher(IRunnable* pRunnable);
    virtual ~CNSAdapter_JSCallDispatcher(void);

    //nsIRunnable
    NS_IMETHOD Run();

private:
    IRunnable* m_pRunnable;
};

#endif //__CNSAdapter_JVMManager_h_
