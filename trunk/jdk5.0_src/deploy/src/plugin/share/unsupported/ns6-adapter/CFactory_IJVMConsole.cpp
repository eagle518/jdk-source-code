#include <stdio.h>
#include "CFactory.h"

NS_IMETHODIMP CFactory::Show() {
    fprintf(stderr,"In Show\n");
    return NS_OK;
}

NS_IMETHODIMP CFactory::Hide() {
    fprintf(stderr,"In Hide\n");
    return NS_OK;
}

NS_IMETHODIMP CFactory::IsVisible(PRBool *result) {
    fprintf(stderr,"In IsVisable\n");
    return NS_OK;
}

NS_IMETHODIMP CFactory::Print(const char* msg, 
                              const char* encodingName) {
    fprintf(stderr,"In Print\n");
    return NS_OK;
}
