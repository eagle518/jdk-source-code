#include <stdio.h>
#include "CFactory.h"


extern "C" NS_EXPORT nsresult NSGetFactory(nsISupports * pProvider,
                                           const nsCID &aClass,
                                           const char * aClassName,
                                           const char * aProgID,
                                           nsIFactory **aFactory) {
    nsresult rv = NS_OK;
    static NS_DEFINE_IID(kIFactoryIID, NS_IFACTORY_IID);
    static NS_DEFINE_CID(kPluginCID, NS_PLUGIN_CID);

    fprintf(stderr,"In NSGetFactory\n");
    
    if (NULL == aFactory) {
       rv = NS_ERROR_UNEXPECTED;
    } else if (aClass.Equals(kPluginCID)) {
       CFactory *fact = new CFactory(pProvider);
       rv = fact->QueryInterface(kIFactoryIID,(void **) aFactory);
    } else {
        rv = NS_ERROR_NO_INTERFACE;
    }

    return rv;
}
