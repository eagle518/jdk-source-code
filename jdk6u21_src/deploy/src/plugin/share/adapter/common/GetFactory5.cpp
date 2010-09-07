/*
 * @(#)GetFactory5.cpp	1.20 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* 
 * GetFactory5.cpp 
 * These functions create, initialize, and shutdown a plugin.
 */

#include "JavaPluginFactory5.h"
#include "nsIFactory.h"
#include "nsIPlugin.h"
#ifdef NS6_ADAPTER
#include "CNS6Adapter_PluginServiceProvider.h"
#include "CNS6Adapter_JavaPluginFactory.h"
#else
#include "CNS7Adapter_PluginServiceProvider.h"
#include "CNS7Adapter_JavaPluginFactory.h"
#endif
#include "CNSAdapter_NSPR.h"
#include "JDSmartPtr.h"
#include "JDSupportUtils.h"
#include "dlfcn.h"

static NS_DEFINE_CID(kPluginCID, NS_PLUGIN_CID);

extern "C" {
#include "Debug.h"
#include <assert.h>
}

extern JDresult LoadNSCore(void** libjpinsp);
/* The very first function called (Unless we are using nsIModule)           
 */                                                                             
extern "C" nsresult NSGetFactory(nsISupports * pProvider,             
                                 const nsCID &aClass,                 
                                 const char * aClassName,             
                                 const char * aProgID,                
                                 nsIFactory **aFactory) {                                               
                                                                             
   UNUSED(aClassName);                                                         
   UNUSED(aProgID);                                                            
 
   nsresult rv = NS_OK;
                                                                               
   if (aFactory == NULL){                                                      
       fprintf(stderr, "Received a null pointer to pointer in NSGetFactory!\n");  
       return NS_ERROR_UNEXPECTED;                                             
   }

   void* libjpinsp = NULL;

   if (LoadNSCore(&libjpinsp) != JD_OK)
     return NS_ERROR_FAILURE;

   if (libjpinsp != NULL && aClass.Equals(kPluginCID)) {
       JDSmartPtr<IFactory> spFactory;
#ifdef NS6_ADAPTER
       JDSmartPtr<IPluginServiceProvider> spPluginSvcProvider(new CNS6Adapter_PluginServiceProvider(pProvider));
#else
       JDSmartPtr<IPluginServiceProvider> spPluginSvcProvider(new CNS7Adapter_PluginServiceProvider(pProvider));
#endif
       if (spPluginSvcProvider == NULL) 
           return NS_ERROR_OUT_OF_MEMORY;

       JD_METHOD (*createPluginFactory)(ISupports* sm,
					IUnixService* us,
					IFactory* *res);

       createPluginFactory = (JD_METHOD (*)(ISupports*, IUnixService*, IFactory**))
			      dlsym(libjpinsp, "createPluginFactory");
       if (createPluginFactory == NULL){                                                      
           fprintf(stderr, "Can't find createPluginFactory symbol !\n");  
           return NS_ERROR_UNEXPECTED;
       }
       
       rv = createPluginFactory(spPluginSvcProvider,
				new CNSAdapter_NSPR,
				&spFactory);

       if (NS_SUCCEEDED(rv) && spFactory) {
#ifdef NS6_ADAPTER
           *aFactory = new CNS6Adapter_JavaPluginFactory(spFactory);
#else
	   *aFactory = new CNS7Adapter_JavaPluginFactory(spFactory);
#endif
           if (*aFactory == NULL)
               return NS_ERROR_OUT_OF_MEMORY;
           (*aFactory)->AddRef();
       }
           
   } else {
       rv = NS_ERROR_NO_INTERFACE;
   } 

   return rv;
}                                                              

