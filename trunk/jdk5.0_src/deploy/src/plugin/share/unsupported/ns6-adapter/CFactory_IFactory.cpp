#include <stdio.h>
#include "CFactory.h"
#include "CPluginInstance.h"
#include "ICreater.h"

NS_IMETHODIMP CFactory::CreateInstance(nsISupports *aOuter, 
                                       const nsIID & iid,
                                       void * *result)
{
fprintf(stderr,"In CreateInstance\n");

    static NS_DEFINE_IID(kIPluginInstanceIID, NS_IPLUGININSTANCE_IID);
    static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

    if (result == NULL) {
        return NS_ERROR_UNEXPECTED;
    }

    *result = NULL;

    if (NULL != aOuter) {
        return NS_ERROR_NO_AGGREGATION;
    }

    if (! (iid.Equals(kIPluginInstanceIID) ||
           iid.Equals(kISupportsIID)))
        return NS_ERROR_NO_INTERFACE;

    ICreater * ic = NULL;
    m_pJavaService->QI(ICreater_IID,(void **) &ic);
    CPluginInstance * pi = new CPluginInstance(ic,m_pServiceManager);
    ic->release();

    return pi->QueryInterface(iid,result);
}

NS_IMETHODIMP CFactory::LockFactory(PRBool lock)
{
fprintf(stderr,"In LockFactory\n");
    return NS_ERROR_NOT_IMPLEMENTED;
}


