#include <stdio.h>
#include "CFactory.h"

NS_IMETHODIMP CFactory::AddToClassPath(const char* dirPath) {
fprintf(stderr,"In AddToClassPath\n");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP CFactory::RemoveFromClassPath(const char* dirPath) {
fprintf(stderr,"In RemoveFromClassPath\n");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP CFactory::GetClassPath(const char* *result) {
fprintf(stderr,"In GetClassPath\n");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP CFactory::GetJavaWrapper(JNIEnv* jenv, jint obj, jobject *jobj) {
fprintf(stderr,"In GetJavaWrapper\n");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP CFactory::CreateSecureEnv(JNIEnv* proxyEnv, 
                                        nsISecureEnv* *outSecureEnv) {
fprintf(stderr,"In CreateSecureEnv\n");

    if (outSecureEnv != NULL) {
        *outSecureEnv = NULL;
    } else {
        return NS_ERROR_UNEXPECTED;
    }

    return NS_ERROR_FAILURE;
}

NS_IMETHODIMP CFactory::SpendTime(PRUint32 timeMillis) {
fprintf(stderr,"In SpendTime\n");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP CFactory::UnwrapJavaWrapper(JNIEnv* jenv, 
                                          jobject jobj, jint* obj) {
fprintf(stderr,"In UnwrapJavaWrapper\n");
    return NS_ERROR_NOT_IMPLEMENTED;
}
