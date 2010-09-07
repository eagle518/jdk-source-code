#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CFactory.h"
#include "adaptercommon.h"

NS_IMETHODIMP CFactory::CreatePluginInstance(nsISupports *aOuter, 
                                REFNSIID aIID,
                                const char* aPluginMIMEType,
                                void **aResult) {
    fprintf(stderr, "CreatePluginInstance\n");
    return CreateInstance(aOuter, aIID, aResult);
}

NS_IMETHODIMP CFactory::Initialize() {
    // Don't do anything here
    fprintf(stderr, "CFactory::Initialize\n");
    return NS_OK;
}

NS_IMETHODIMP CFactory::Shutdown(void) {
    fprintf(stderr, "in Factory shutdown\n");
    return NS_OK;
}

NS_IMETHODIMP CFactory::GetMIMEDescription(const char* *resultingDesc) {
   
    char * table;
    const char * vmString = ";application/x-java-vm::Java(tm) Plug-in";

    fprintf(stderr,"In GetMIMEDescription\n");
    
    ac_createMimeTable(m_pJavaService,&table);
    
    *resultingDesc = (const char*) malloc(strlen(table) + strlen(vmString) +1);
   
    strcpy((char *) *resultingDesc,table); 
    strcat((char *) *resultingDesc,vmString);
    
    return NS_OK;
}

NS_IMETHODIMP CFactory::GetValue(nsPluginVariable variable, void *value) {

    static char pluginNameString[100];
    nsresult err = NS_OK;

    switch (variable) {
      case nsPluginVariable_NameString:
          /* Can there be a race here ? */
          if (pluginNameString[0] == 0)
              sprintf(pluginNameString,"Java(TM) Plug-in %s", VERSION);
          *((char **)value) = pluginNameString;
          break;

      case nsPluginVariable_DescriptionString:
        *((const char **)value) = "Java(TM) Plug-in "PLUGIN_VERSION;
        break;

      default:
        err = NS_ERROR_ILLEGAL_VALUE;
    }

    return err;
}
