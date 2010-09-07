/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__CPluginInstance__)
#define __CPluginInstance__

#include "nsplugin.h"
#include "nsIJVMPluginInstance.h"
#include "nsIServiceManager.h"
#include "nsIPluginInstance.h"
#include "IJavaInstanceCB.h"
#include "ICreater.h"
#include "IJavaInstance.h"

class CPluginInstance : public nsIJVMPluginInstance, nsIPluginInstance,
                               IJavaInstanceCB {
public:
    NS_DECL_ISUPPORTS

    NS_DECL_NSIJVMPLUGININSTANCE

    // nsPluginInstance methods
    NS_IMETHOD Initialize(nsIPluginInstancePeer* peer);
    NS_IMETHOD GetPeer(nsIPluginInstancePeer* *resultingPeer);
    NS_IMETHOD Start(void);
    NS_IMETHOD Stop(void);
    NS_IMETHOD Destroy(void);
    NS_IMETHOD SetWindow(nsPluginWindow* window);
    NS_IMETHOD NewStream(nsIPluginStreamListener** listener);
    NS_IMETHOD Print(nsPluginPrint* platformPrint);
    NS_IMETHOD GetValue(nsPluginInstanceVariable variable, void *value);
    NS_IMETHOD HandleEvent(nsPluginEvent* event, PRBool* handled);

    DECL_IEGO

    // IJavaInstanceCB methods
    void showStatus(char * ); 
    void showDocument(char *, char *);
    void findProxy(char *, char **);
    void findCookie(char *, char **);
    void javascriptRequest(char *);
    void setCookie(char *, char *);

    // constructors and destructor
    CPluginInstance(ICreater *, nsIServiceManager *);
    virtual ~CPluginInstance();

private:
    nsIPluginInstancePeer* m_PluginInstancePeer;
    nsIServiceManager * m_pServiceManager;
    ICreater * m_Creater;
    IJavaInstance * m_JavaInstance;
    bool m_destroyed;
};

#endif
