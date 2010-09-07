#include <stdio.h>
#include "CPluginInstance.h"

NS_IMETHODIMP CPluginInstance::QueryInterface(const nsIID & iid, void **ptr)
{
    static NS_DEFINE_IID(kIPluginInstanceIID, NS_IPLUGININSTANCE_IID);
    static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
    static NS_DEFINE_IID(kIJVMPluginInstanceIID, NS_IJVMPLUGININSTANCE_IID);

    fprintf(stderr,"In CPluginInstance::QueryInterface\n");
    if (NULL == ptr) {
        return NS_ERROR_NULL_POINTER;
    } 

    if(iid.Equals(kISupportsIID) || iid.Equals(kIJVMPluginInstanceIID)) {
        *ptr = (void*) (nsIJVMPluginInstance*) this;
    } else if (iid.Equals(kIPluginInstanceIID)) {
        *ptr = (void*) (nsIPluginInstance*) this;
    } else {
        return NS_NOINTERFACE;
    }
    
    AddRef();
    return NS_OK;
}

NS_IMPL_ADDREF(CPluginInstance)
NS_IMPL_RELEASE(CPluginInstance)

