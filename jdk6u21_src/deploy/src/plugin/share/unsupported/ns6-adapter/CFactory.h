/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__CFactory__)
#define __CFactory__

#include "nsplugin.h"
#include "nsIJVMPlugin.h"
#include "nsIJVMConsole.h"
#include "nsIServiceManager.h"

#include "IEgo.h"

class CFactory : public nsIJVMPlugin, nsIPlugin, nsIJVMConsole {
public:

    NS_DECL_ISUPPORTS

    // nsIJVMPlugin
    NS_IMETHOD AddToClassPath(const char* dirPath);
    NS_IMETHOD RemoveFromClassPath(const char* dirPath);
    NS_IMETHOD GetClassPath(const char* *result);
    NS_IMETHOD GetJavaWrapper(JNIEnv* jenv, jint obj, jobject *jobj);
    NS_IMETHOD CreateSecureEnv(JNIEnv* proxyEnv, nsISecureEnv* *outSecureEnv);
    NS_IMETHOD SpendTime(PRUint32 timeMillis);
    NS_IMETHOD UnwrapJavaWrapper(JNIEnv* jenv, jobject jobj, jint* obj);

    // nsIPlugin
    NS_IMETHOD CreatePluginInstance(nsISupports *aOuter, REFNSIID aIID,
                                    const char* aPluginMIMEType,
                                    void **aResult);
    NS_IMETHOD Initialize();
    NS_IMETHOD Shutdown(void);
    NS_IMETHOD GetMIMEDescription(const char* *resultingDesc);
    NS_IMETHOD GetValue(nsPluginVariable variable, void *value);

    // nsIFactory
    NS_DECL_NSIFACTORY

    // nsIJVMConsole
    NS_IMETHOD Show(void);
    NS_IMETHOD Hide(void);
    NS_IMETHOD IsVisible(PRBool *result);
    NS_IMETHOD Print(const char* msg, const char* encodingName = NULL);


    // constructors and destructor
    CFactory(nsISupports *);
    virtual ~CFactory();

private:
    nsIServiceManager * m_pServiceManager;
    IEgo * m_pJavaService;
};

#endif
