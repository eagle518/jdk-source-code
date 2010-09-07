#include <stdio.h>
#include "CPluginInstance.h"

NS_IMETHODIMP CPluginInstance::GetJavaObject(jobject *result)
{
fprintf(stderr,"CPluginInstance::GetJavaObject\n");
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP CPluginInstance::GetText(const char ** result)
{
fprintf(stderr,"CPluginInstance::GetText\n");
    return NS_ERROR_NOT_IMPLEMENTED;
}

