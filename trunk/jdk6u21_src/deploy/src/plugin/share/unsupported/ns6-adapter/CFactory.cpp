#include <stdio.h>
#include "CFactory.h"
#include "adaptercommon.h"
#include "nsIPluginManager2.h"
#include "ICreater.h"
#include "CFDMonitor.h"

CFactory::CFactory(nsISupports * pProvider) {
   
    static NS_DEFINE_IID(kIPluginManager2IID, NS_IPLUGINMANAGER2_IID);
    static NS_DEFINE_IID(kIServiceManagerIID, NS_ISERVICEMANAGER_IID);
    static NS_DEFINE_CID(kCPluginManagerCID, NS_PLUGINMANAGER_CID);
 
    NS_INIT_REFCNT();

    nsresult rv = pProvider->QueryInterface(kIServiceManagerIID,
                                             (void **) &m_pServiceManager);
    if (NS_SUCCEEDED(rv)) {
        nsIPluginManager2 * pluginManager;
        rv = m_pServiceManager->GetService(kCPluginManagerCID,
                                           kIPluginManager2IID,
                                           (nsISupports **) &pluginManager);
        if (NS_SUCCEEDED(rv)) {
            const char * agent = "No agent";
            rv = pluginManager->UserAgent(&agent);
            if (NS_SUCCEEDED(rv)) {
                ICreater * ic;
                ac_createJavaService(agent, (void **) &m_pJavaService);
                m_pJavaService->QI(ICreater_IID,(void **) &ic);
                ic->setFDMonitor((IFDMonitor *) new CFDMonitor());
            }
            pluginManager->Release();
        }
    } else {
        fprintf(stderr, "Nope Guess Agian!!\n");
    }

}

CFactory::~CFactory() {

    fprintf(stderr,"In ~CFactory\n");
    if (m_pServiceManager != NULL) {
        m_pServiceManager->Release();
    }
}

