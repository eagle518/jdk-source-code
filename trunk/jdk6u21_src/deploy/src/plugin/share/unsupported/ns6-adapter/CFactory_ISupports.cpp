#include <stdio.h>
#include "CFactory.h"

NS_IMETHODIMP CFactory::QueryInterface(const nsIID & iid, void **ptr)
{
    fprintf(stderr,"In QueryInterface\n");

    static NS_DEFINE_IID(kIPluginIID, NS_IPLUGIN_IID);
    static NS_DEFINE_IID(kIFactoryIID, NS_IFACTORY_IID);
    static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
    static NS_DEFINE_IID(kIJVMPluginIID, NS_IJVMPLUGIN_IID);
    static NS_DEFINE_IID(kIJVMConsoleIID, NS_IJVMCONSOLE_IID);

    if (NULL == ptr) {
        return NS_ERROR_NULL_POINTER;
    } 

    if(iid.Equals(kISupportsIID) || iid.Equals(kIJVMPluginIID)) {
        *ptr = (void*) (nsIJVMPlugin*) this;
    } else if (iid.Equals(kIPluginIID)) {
        *ptr = (void*) (nsIPlugin*) this;
    } else if (iid.Equals(kIFactoryIID)) {
        *ptr = (void*) (nsIFactory*) this;
    } else if (iid.Equals(kIJVMConsoleIID)) {
fprintf(stderr,"Looking for the Console Interface\n");
        *ptr = (void*) (nsIJVMConsole*) this;
    } else {
        return NS_NOINTERFACE;
    }
    
    AddRef();
    return NS_OK;
}

NS_IMPL_ADDREF(CFactory)
NS_IMPL_RELEASE(CFactory)

