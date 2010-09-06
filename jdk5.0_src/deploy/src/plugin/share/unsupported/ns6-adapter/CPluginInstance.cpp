#include <stdio.h>
#include "CPluginInstance.h"

CPluginInstance::CPluginInstance(ICreater * ic, 
                                 nsIServiceManager * pServiceManager) {
   
    fprintf(stderr,"In CPluginInstance\n");
    NS_INIT_REFCNT();
    m_PluginInstancePeer = NULL;

    m_Creater = ic;
    m_Creater->add();
  
    m_pServiceManager = pServiceManager;
    m_pServiceManager->AddRef();

    m_JavaInstance = NULL;
    m_destroyed = false;
}

CPluginInstance::~CPluginInstance() {

    fprintf(stderr,"In ~CPluginInstance\n");
    if (NULL != m_PluginInstancePeer) {
        m_PluginInstancePeer->Release();
    }

    if (NULL != m_JavaInstance) {
        m_JavaInstance->release();
    }

    m_Creater->release();
}

