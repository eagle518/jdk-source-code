#include <stdio.h>
#include "CPluginInstance.h"

JRESULT CPluginInstance::QI(const IID& iid, void ** ppv) {
    if (iid == IEgo_IID || iid == IJavaInstanceCB_IID) {
        *ppv = static_cast<IJavaInstanceCB *>(this);
        *ppv = NULL;
        return J_NOINTERFACE;
    }

    reinterpret_cast<IEgo *>(*ppv)->add();
    return J_NOERROR;
}

// Special case.  We use the Netscape refcount as the single
// refcount for this object 

unsigned long CPluginInstance::add() {

    return this->AddRef();
}

unsigned long CPluginInstance::release() {

    return this->Release();
}

