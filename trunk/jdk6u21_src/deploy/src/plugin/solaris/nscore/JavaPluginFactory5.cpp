/*
 * @(#)JavaPluginFactory5.cpp	1.61 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* 
 * Version of the plugin factory that works with the 5.0 version of
 * the browser. Provides a function pointer table that is used
 * by the main PluginFactory. 
 * The plugin factory is responsible for all the main plugin fuctionality.
 * It delegates to two other classes - the ProxySupport object for help
 * with interfacing to proxies and the JavaVM for handling the
 * actual link to the out-of-proc VM
 */


#include "commonhdr.h"
#include "IPlugin.h"
#include "IJVMPlugin.h"
#include "IPluginInstance.h"
#include "IJVMConsole.h"
#include "IPluginServiceProvider.h"
#include "IJVMManager.h"
#include "IPluginManager.h"
#include "ILiveconnect.h"
#include "ICookieStorage.h"
#include "ISecureEnv.h"
#include "IUnixService.h"
#include "commandprotocol.h"
#include "plugin_defs.h"
#include "pluginversion.h"
#include "remotejni.h"
#include "JavaPluginInstance5.h"
#include "CJavaConsole.h"
#include "CSecureJNIEnv.h"
#include "JavaVM5.h"
#include "CookieSupport.h"
#include "ProxySupport5.h"
#include "JavaPluginFactory5.h"

extern "C" {
#include <assert.h>
}

/* Global plugin factory */
static JavaPluginFactory5* g_plugin_factory = NULL;
extern IUnixService*    g_unixService = NULL;

JavaPluginFactory5* get_global_factory() {
  if (g_plugin_factory == NULL) {
	plugin_error("No global plugin factory!");
  }
    return  g_plugin_factory;
}

JD_METHOD createPluginFactory(ISupports* sm,
			      IUnixService* us,
			      IFactory* *res) {
  trace("JavaPluginFactory5::createPluginFactory:\n");

  if (sm == NULL || res == NULL) {
    plugin_error("NULL pointer received when initializing factory!");
  }

  if (us == NULL) {
    plugin_error("No Unix Service!");
  }

  g_unixService = us;

  return JavaPluginFactory5::Create(sm, 
				   JD_GET_IID(IFactory),
				   (void**)res);
}

JD_DEFINE_IID(jIPluginIID, IPLUGIN_IID);
JD_DEFINE_IID(jIPluginInstanceIID, IPLUGININSTANCE_IID);
JD_DEFINE_IID(jIFactoryIID, IFACTORY_IID);
JD_DEFINE_IID(jISupportsIID, ISUPPORTS_IID);
JD_DEFINE_IID(jIJVMConsoleIID, IJVMCONSOLE_IID);
JD_DEFINE_IID(jILiveConnectIID, ILIVECONNECT_IID);
JD_DEFINE_IID(jIPluginServiceProviderIID, IPLUGINSERVICEPROVIDER_IID);
JD_DEFINE_IID(jIPluginManagerIID, IPLUGINMANAGER_IID);
JD_DEFINE_IID(jIJVMPluginIID, IJVMPLUGIN_IID);
JD_DEFINE_IID(jIJVMManagerIID, IJVMMANAGER_IID);
JD_DEFINE_IID(jICookieStorageIID, ICOOKIESTORAGE_IID);

JD_DEFINE_CID(jCLiveConnectCID, CLIVECONNECT_CID);
JD_DEFINE_CID(jCJVMManagerCID, CJVMMANAGER_CID);
JD_DEFINE_CID(jCPluginManagerCID, CPLUGINMANAGER_CID);

JD_METHOD JavaPluginFactory5::Create(ISupports* sm,   
				     const JDIID& aIID,    
				     void* *aInstancePtr) {

    JDresult rv = JD_ERROR_FAILURE;
    if (aInstancePtr == NULL) {
        plugin_error("Received a null pointer to pointer in Create!\n");
    } else {
        JavaPluginFactory5 *res;

        if (!g_plugin_factory) {
	    IPluginServiceProvider* spService;
	    if (JD_FAILED(sm->QueryInterface(jIPluginServiceProviderIID, (void**)&spService)))
	        return rv;
	    
            res = new JavaPluginFactory5(spService); // g_plugin_factory set in here
            // The peice of shit browser does not seem to call Initialize
            // on this code path!!!
            spService->Release();
	    res->Initialize(); 
            init_utils();
        } else {
            res = g_plugin_factory;
        }
        rv = res->QueryInterface(aIID,aInstancePtr);
    }
    return rv;
}

/*
 * Performs simple initialization of various variables. This version
 * can dump the hacks from previous versions - it only has to work
 * with the 5.0 browser
 */
JavaPluginFactory5::JavaPluginFactory5(IPluginServiceProvider* pProvider)  {
    trace("JavaPluginFactory5:Constructor\n");
    JD_INIT_REFCNT();
    plugin_manager = NULL;
    jvm_manager = NULL;
    cookieStorage = NULL;
    isInitialized = false;
    is_java_vm_started = 0;
    g_plugin_factory = this;
    factory_monitor = g_unixService->JD_NewMonitor();
    javaVM = new JavaVM5(this);
    proxy_support = new ProxySupport5(javaVM);
    cookieSupport = new CookieSupport(javaVM);
    /* Allocate and zero out array  memory since NULL is used to 
       indicate an empty spot */
    int inst_sz = PLUGIN_INSTANCE_COUNT * sizeof(JavaPluginInstance5 *);
    plugin_instances = (JavaPluginInstance5 **)	malloc(inst_sz);
    memset(plugin_instances, 0, inst_sz);

    int locked_inst_sz = PLUGIN_INSTANCE_COUNT * sizeof(bool);
    locked_plugin_instances = (bool*) malloc(locked_inst_sz);
    memset(locked_plugin_instances, 0, locked_inst_sz);

    int env_sz = MAX_ENVS * sizeof(RemoteJNIEnv*);
    int proxy_env_sz = MAX_ENVS * sizeof(JNIEnv*);
    current_envs = (RemoteJNIEnv**) malloc(env_sz);
    current_proxy_envs = (JNIEnv**) malloc(proxy_env_sz);
    memset(current_envs, 0, env_sz);
    memset(current_proxy_envs, 0, proxy_env_sz);
    pluginNameString=(char*)malloc(100);
    memset(pluginNameString, 0, sizeof(pluginNameString));
    service_provider = pProvider;
    if (service_provider)
      service_provider->AddRef();

    CJavaConsole::Create((IJVMPlugin*)this, this, jISupportsIID, (void**)&m_pIJVMConsole);
}

//nsISupports interface methods. 
// Mozilla changes : Signature changed
JD_IMETHODIMP JavaPluginFactory5::QueryInterface(const JDIID & iid, void **ptr) {
    if (NULL == ptr)                                         
	return JD_ERROR_NULL_POINTER;  
    
    if (iid.Equals(jIJVMPluginIID) || 
	iid.Equals(jISupportsIID)) {
      *ptr = (void*) (IJVMPlugin*) this;
      AddRef();
      return JD_OK;		
   }
   else if (iid.Equals(jIPluginIID) || 
	    iid.Equals(jIFactoryIID)) {
     *ptr = (void*) (IPlugin*) this;
     AddRef();
     return JD_OK;		
   }
   else if (iid.Equals(jIJVMConsoleIID))  {
     return m_pIJVMConsole->QueryInterface(iid, ptr);
   }
   else
     return JD_NOINTERFACE;
}

JD_IMPL_ADDREF(JavaPluginFactory5) 
JD_IMPL_RELEASE(JavaPluginFactory5)

// nsIPlugin methods


// (Corresponds to NPP_GetMIMEDescription.)
JD_IMETHODIMP JavaPluginFactory5::GetMIMEDescription(const char **resDesc) 
{
    trace("JavaPluginFactory::GetMIMEDescription\n");
    *resDesc = "application/x-java-vm::Java&#153 Plug-in;"PLUGIN_MIMETABLE;

    return JD_OK;
}


/* LockFactory does nothing at the moment */
JD_IMETHODIMP JavaPluginFactory5::LockFactory(JDBool aLock) {
    trace("LockFactory %d\n", (int) aLock);
    return JD_OK;
}

JavaVM5*
JavaPluginFactory5::GetJavaVM(void) {
  if (!is_java_vm_started) {
    JVMInitArgs args;
    args.version = JVMInitArgs_Version;
    args.classpathAdditions = NULL;
    JDresult ret = StartupJVM(&args);
    if (JD_OK != ret) {
      plugin_formal_error("VM did not start up properly");
      is_java_vm_started = 0;
      return NULL;
    }
  }
  return javaVM;
}

JavaPluginFactory5::~JavaPluginFactory5(void)  {
    trace("JavaPluginFactory5:******************** DESTROYING THE PLUGIN FACTORY! ***********\n");

    g_plugin_factory = NULL;
      
    if (plugin_manager != NULL)
      plugin_manager->Release();

    if (is_java_vm_started)
      ShutdownJVM(0);
    
    if (javaVM) 
     delete(javaVM);
    if (proxy_support)
     delete(proxy_support);
    if (plugin_instances)
      free(plugin_instances);
    if (locked_plugin_instances)
      free(locked_plugin_instances);
    if (cookieSupport)
      delete(cookieSupport);
    if(pluginNameString != NULL)
      free(pluginNameString);

    if (service_provider)
      service_provider->Release();

    //XXXFIX Destroy the service provider? mProvider
  
    delete(g_unixService);
    g_unixService = NULL;
    /* 
    if (m_pIJVMConsole != NULL) 
	m_pIJVMConsole->Release;
    */
       
}

JD_IMETHODIMP
JavaPluginFactory5::Initialize() {
  // With the new API, the service manager
  // made available in the GetFactory method and is 
  // then passed into the constructor of the factory,
  // where the CPluginServiceProvider is created.
  // No args to initialize
  
    static JDresult error = JD_ERROR_FAILURE;
    if (isInitialized) 
        return error;
    else 
        isInitialized = true;
    trace("JavaPluginFactory5::Initialize\n");

    /* Set the plugin and jvm managers */
    //XXXFIX consider the lifetime of these objects and when
    // the refs should be released. They should probably
    // be freed when this object is destroyed, but there might
    // be circular references from the plugin manager to the
    // plugin factory and back.
    if (JD_FAILED(service_provider->QueryService(jCPluginManagerCID, 
						 jIPluginManagerIID, 
						 (ISupports **)&plugin_manager)))
      plugin_error("Could not get the plugin manager");
    
    if (plugin_manager != NULL) {
	/* Dump the environment variables for debugging */
	if (tracing) {
	    char *cp = getenv("CLASSPATH");
	    char *jtrace = getenv("JAVA_PLUGIN_TRACE");
	    char *vmwait  = getenv("JAVA_VM_WAIT");
	    char *ldpath = getenv("LD_LIBRARY_PATH");
	    if (cp) trace("CLASSPATH = %s\n", cp);
	    if (jtrace) trace("JAVA_PLUGIN_TRACE = %s\n", jtrace);
	    if (vmwait) trace("JAVA_VM_WAIT = %s\n", vmwait);
	    if (ldpath) trace("LD_LIBRARY_PATH = %s\n", ldpath);
	}
	return error;
    } else {
	plugin_error("No manager for initializing factory?\n");
	error = JD_ERROR_ILLEGAL_VALUE;
	return error;
    }

}


JD_IMETHODIMP
JavaPluginFactory5::Shutdown(void) {
    trace("JavaPluginFactory5:shutdown\n");


    if (plugin_manager != NULL) {
	plugin_manager->Release();
	plugin_manager = NULL;
    }

    return JD_OK;
}


// Mozilla changes : Probably should be doing more here...
JD_IMETHODIMP
JavaPluginFactory5::CreatePluginInstance(ISupports *aOuter,
                                         JDREFNSIID aIID,
                                         const char * aMineType,
                                         void **result) {
  UNUSED (aMineType);
  trace("JavaPluginFactory5:CreatePluginInstance\n");
  return CreateInstance(aOuter, aIID, result);
}

JD_IMETHODIMP
JavaPluginFactory5::CreateInstance(ISupports *aOuter,
				   const JDIID & aIID,
				   void **result) {
    trace("JavaPluginFactory5:CreateInstance\n");

    if (result == NULL) {
        plugin_error("NULL result in create instance");
	return JD_ERROR_NULL_POINTER;
    }

    *result = NULL;

    if (aOuter != NULL) {
        plugin_error("NO_AGGREGATION in create instance!");
	return JD_ERROR_NO_AGGREGATION;
    }

    if (! (aIID.Equals(jIPluginInstanceIID) ||
	   aIID.Equals(jISupportsIID)))
	return JD_NOINTERFACE;

    /* Startup the JVM if it is not already running */
    JavaVM5* vm = GetJavaVM();

    if (!is_java_vm_started) return JD_ERROR_FAILURE;
    
    /* Create a new instance and refcount it */
    JavaPluginInstance5 * pluginInstance = new JavaPluginInstance5(this);

    *result = (IPluginInstance *) pluginInstance;
    pluginInstance->AddRef();

    UNUSED(vm);

    return JD_OK;

}


JD_IMETHODIMP
JavaPluginFactory5::StartupJVM(JVMInitArgs *initargs) {
    trace("JavaPluginFactory5:StartupJVM\n");

    JDresult ret = JD_OK;

    if (is_java_vm_started) {
	plugin_error("StartupJVM is being called twice!\n");
	return JD_OK;
    }

    EnterMonitor("StartupJVM");
    
    /* Make sure someone did not start it up while we were
       waiting for the monitor */
    if (!is_java_vm_started) {
	ret = javaVM->StartJavaVM(initargs->classpathAdditions);

	if (ret == JD_OK) 
	    is_java_vm_started = 1;
	 else
	     plugin_formal_error("Could not start JavaVM!\n");
    } else {
	plugin_error("StartupJVM has already been called.\n");
    }

    ExitMonitor("StartupJVM");

    return ret;

}


JD_IMETHODIMP
JavaPluginFactory5::ShutdownJVM(JDBool fullShutdown) {
    trace("JavaPluginFactory5:ShutdownJVM\n");

    javaVM->ShutdownJavaVM(fullShutdown);
    is_java_vm_started = 0;
    return JD_OK;
}


JD_IMETHODIMP
JavaPluginFactory5::GetValue(JDPluginVariable variable, void *value) {
    trace("JavaPluginFactory5:GetValue\n");

    JDresult err = JD_OK;

    switch (variable) {
      case JDPluginVariable_NameString:
	  /* Can there be a race here ? */
	  if (pluginNameString[0] == 0)
	      sprintf(pluginNameString,"Java(TM) Plug-in %s", VERSION);
	  *((char **)value) = pluginNameString;
	  break;

      case JDPluginVariable_DescriptionString:
	*((const char **)value) = "Java(TM) Plug-in "PLUGIN_VERSION;
	break;

      default:
	err = JD_ERROR_ILLEGAL_VALUE;
    }

    return err;

}

JD_IMETHODIMP
JavaPluginFactory5::GetClassPath(const char* *result) {
    trace("JavaPluginFactory5:GetClassPath\n");
    *result = getenv("CLASSPATH");
    return JD_OK;
}

JD_IMETHODIMP
JavaPluginFactory5::GetJavaWrapper(JNIEnv* proxy_env, 
				   jint browser_obj, 
				   jobject* java_obj) {
    trace("JavaPluginFactory5:JavaPluginFactory5::GetJavaWrapper()\n");

    /* Can determine the right RemoteJNIEnv from looking up the
       env for the current thread, or can look up the remote
       JNIEnvs to find out which one corresponds to this proxyenv */

    if (browser_obj == 0 || java_obj == NULL)    
	return JD_ERROR_NULL_POINTER;
    
    RemoteJNIEnv* env = GetRemoteEnv(proxy_env);
    env->ExceptionClear();
    jclass jsobj5_clazz = 
    env->FindClass("sun/plugin/javascript/navig5/JSObject");

    if (jsobj5_clazz == NULL) 
      plugin_error("Could not create the java wrapper. No JSObject\n");

    jmethodID jsobj_create_method = env->GetMethodID(jsobj5_clazz, 
						     "<init>",
						     "(II)V");
    *java_obj = env->NewObject(jsobj5_clazz, jsobj_create_method,
			      (jint)(g_unixService->JD_GetCurrentThread()), (jint) browser_obj);
    return JD_OK;
}

JD_IMETHODIMP
JavaPluginFactory5::UnwrapJavaWrapper(JNIEnv* jenv, jobject jobj, jint* obj) {

    // I copied this function from the Windows version and 
    // modified it to use the remote JNI (I hope [smk])

    trace("JavaPluginFactory5::UnwrapJavaWrapper\n");

    if (jenv == NULL || jobj == NULL || obj == NULL)
        return JD_ERROR_NULL_POINTER;

    RemoteJNIEnv* env = GetRemoteEnv(jenv);
    env->ExceptionClear();

    // Find class
    jclass clazz = env->FindClass("sun/plugin/javascript/navig5/JSObject");

    // Check if we may unwrap the JSObject
    if (env->IsInstanceOf(jobj, clazz) == JNI_FALSE)
    {
        return JD_ERROR_FAILURE;
    }

    jfieldID fid = env->GetFieldID(clazz, "nativeJSObject", "I");
    *obj = env->GetIntField(jobj, fid);

    return JD_OK;
}


/* Create a new applet, with a particular index */
void
JavaPluginFactory5::CreateApplet(const char* appletType, int appletNumber,
				 int argc, char **argn, char **argv) {
    trace("JavaPluginFactory5:CreateApplet\n");

    /* Not sure if calling create applet is permitted before startup */
    if (!is_java_vm_started) {
	plugin_formal_error("CreateApplet called before the VM is started\n?");
    }

    EnterMonitor("CreateApplet");

    /* The VM could have been shutdown again */
    if (is_java_vm_started) {
	trace("JavaPluginFactory5::CreateApplet %d \n", appletNumber);
	javaVM->CreateApplet(appletType, appletNumber, argc, argn, argv);
    } else {
	plugin_formal_error("VM not initialized. Cannot create applet!");
    }

    ExitMonitor("CreateApplet");
	
}


JD_IMETHODIMP
JavaPluginFactory5::CreateSecureEnv(JNIEnv* proxy_env, ISecureEnv* *result) 
{
    trace("JavaPluginFactory5:CreateSecureEnv\n");
    if (result != NULL) *result = NULL;

// This is left here to make it easy to turn off live connect support 
// should something prove to be out of wack! (Just un comment)
//    return JD_ERROR_FAILURE;

    JavaVM5 * vm = GetJavaVM();

    if (!is_java_vm_started) return JD_ERROR_FAILURE;

    if(!(vm->GetJVMEnabled())) return JD_ERROR_FAILURE;

    RemoteJNIEnv* remote_env = javaVM->CreateRemoteJNIEnv(proxy_env);

    return CSecureJNIEnv::Create(NULL, remote_env, 
			         jISupportsIID, 
			         (void**) result);
}


/* Register an env and return an index associated with that env. 
   This index should not be used directly, but may be useful for 
   debugging purposes. We also associate the current thread with
   'env' */
int 
JavaPluginFactory5::RegisterRemoteEnv(RemoteJNIEnv* env, JNIEnv* proxy_env) {
    EnterMonitor("Register Env");
    for(int i = 0; i < MAX_ENVS; i++) {
	if (current_envs[i] == NULL) {
	    current_envs[i] = env;
	    current_proxy_envs[i] = proxy_env;
	    trace("JavaPluginFactory5: Register Env [%d] proxyenv=%d\n", i, (int) proxy_env);
	    ExitMonitor("Register Env");
	    return i;
	}
    }
    ExitMonitor("Register Env");
    plugin_error("Env table is full!");
    return -1;
}

/* Free the index assocaited with the remote env 'env'. Should be called
   when the remote env is destroyed, which should be when the securejni
   is destroyed */
int 
JavaPluginFactory5::UnregisterRemoteEnv(RemoteJNIEnv* env) {
    EnterMonitor("Register Env");
    for(int i = 0; i < MAX_ENVS; i++) {
	if (current_envs[i] == env) {
	    current_envs[i] = NULL;
	    current_proxy_envs[i] = NULL;
	    ExitMonitor("Register Env");
	    return i;
	}
    }
    ExitMonitor("Register Env");
    plugin_error("No such env found!");
    return -1;
}

/* The folowing methods are not locked since there should be no race
   between modifying and reading the remote env table i.e.
   Looking up an env by index must always work (or there's a bug)
   Looking up an env by the current thread  
     - if there is no env for the thread, then only the current thread
        can remove it and vice versa.
   */
RemoteJNIEnv*
JavaPluginFactory5::GetRemoteEnv(JNIEnv* proxy_env) {
    for(int i = 0; i < MAX_ENVS; i++) {
	if (current_proxy_envs[i] == proxy_env) {
	  return current_envs[i];
	}
    }
    plugin_error("No remote env found for the proxy_env\n");
    return NULL;
}

/*  
 * Plugin Internal: Register an instance with the plugin We would like
 * to be able to recover PluginInstances from the plugin.  So we keep
 * a registry of currently running instances. We provide the functions
 * to register, unregister, and get a plugin instance from that
 * registry.
 */
int 
JavaPluginFactory5::RegisterInstance(JavaPluginInstance5* pluginInstance) {
    trace("JavaPluginFactory5:RegisterInstance\n");
    int i;

    EnterMonitor("RegisterInstance");
    for (i = 0; i < PLUGIN_INSTANCE_COUNT; i++) {
        /* Check if the plugin is valid and not in used */
	if (plugin_instances[i] == NULL &&
	    !locked_plugin_instances[i]) {
	    trace("JavaPluginFactory5::RegisterInstance %d at %d\n", 
		  (int) pluginInstance, i);
	    plugin_instances[i] = pluginInstance;
	    ExitMonitor("RegisterInstance");
	    return i;
	}
    }
    plugin_error("Could not register plugininstance\n");
    ExitMonitor("RegisterInstance");
    return JD_ERROR_FAILURE;
}

/* 
 * Plugin Internal: Unregister an instance from the internal table.
 */
void
JavaPluginFactory5::UnregisterInstance(JavaPluginInstance5 * pluginInstance) {
    int i;
    EnterMonitor("UnregisterInstance.");
    int plugin_number = pluginInstance->GetPluginNumber();
    trace("JavaPluginFactory5::Unregistering %d \n", plugin_number);
    for (i = 0; i < PLUGIN_INSTANCE_COUNT; i++) {
	if (plugin_instances[i] == pluginInstance) {	
	    trace("JavaPluginFactory::Unregistering instance %d\n", i);
	    plugin_instances[i] = NULL;
	    ExitMonitor("UnregisterInstance-a");
	    return;
	}
    }
    trace("JavaPluginFactory5::Plugin: couldn't find plugin_instance %d\n", plugin_number);
    ExitMonitor("UnregisterInstance-b");
    return;
}
   
JavaPluginInstance5 *JavaPluginFactory5::GetInstance(int index) {
    trace("JavaPluginFactory5:GetInstance\n");

    JavaPluginInstance5* res;
    EnterMonitor("GetInstance");
    
    /* First handle the -1 index - pick a random instance */
    if (index == -1) {
	for (int i = 0; i < PLUGIN_INSTANCE_COUNT; i++) {
	    if (plugin_instances[i] != (JavaPluginInstance5 *)NULL) {
		if (tracing)
		    trace("JavaPluginFactory5::Chose random instance %d\n", i);
		ExitMonitor("GetInstance-any");
		return plugin_instances[i];
	    }
	}
        trace("JavaPluginFactory5:Returning NULL for random instance");
        return (JavaPluginInstance5 *)NULL; 
    }

    /* For a non-random index, check bounds */
    if ((index < 0) || (index >= PLUGIN_INSTANCE_COUNT)) {
	plugin_error("Plugin instance index out of bounds %d\n", index);
	res =  (JavaPluginInstance5 *)NULL;
    } else {
	res = (JavaPluginInstance5 *) plugin_instances[index];    
	if (res == NULL) 
	    trace("JavaPluginFactory::CreateInstance Returning a NULL instance! %d\n", index);
    }
    ExitMonitor("GetInstance-normal");

    return res;
}


/*  
 * Plugin Internal: Indicate that a particular plugin instance index
 * may be in use by the java_vm process and should not be reused yet.
 */
void
JavaPluginFactory5::LockInstanceIndex(int index) {
    trace("JavaPluginFactory5:LockInstanceIndex\n");

    EnterMonitor("LockInstanceIndex");
    locked_plugin_instances[index] = true;
    ExitMonitor("LockInstanceIndex");
}


/*  
 * Plugin Internal: Indicate that a particular plugin instance index
 * is no longer in use by the java_vm process and may be reused.
 */
void
JavaPluginFactory5::UnlockInstanceIndex(int index) {
    trace("JavaPluginFactory5:UnlockInstanceIndex\n");

    EnterMonitor("UnlockInstanceIndex");
    locked_plugin_instances[index] = false;
    ExitMonitor("UnlockInstanceIndex");
}


/* Make a call to return the plugin instance for an object */
jobject 
JavaPluginFactory5::GetJavaObjectForInstance(int plugin_number) {
  /* Get the vm, making sure it is started if necessary */
  JavaVM5* jvm = GetJavaVM();
  
  if (!is_java_vm_started) return NULL;
  
  return jvm->GetJavaObjectForInstance(plugin_number);

}

void JavaPluginFactory5::SendRequest(const CWriteBuffer& wb, int wait_for_reply)
{
    EnterMonitor("SendRequest");
    if (is_java_vm_started)
	javaVM->SendRequest(wb, wait_for_reply);
    else {
	plugin_formal_error("VM is not yet started up in SendRequest!");
    }
    ExitMonitor("SendRequest");
}


ProxySupport5* JavaPluginFactory5::GetProxySupport(void) {
    trace("JavaPluginFactory5:GetProxySupport");
    if (proxy_support == NULL) {
	plugin_error("Proxy support is null!");
    }
    return proxy_support;
}


void JavaPluginFactory5::EnterMonitor(const char *msg) {
    if (tracing)
	trace("JavaPluginFactory5 trying to enter %s\n", msg);

    g_unixService->JD_EnterMonitor(factory_monitor);

    if (tracing)
	trace("JavaPluginFactory5 Entered %s\n", msg);

}


void JavaPluginFactory5::ExitMonitor(const char *msg) {
    if (tracing)
	trace("JavaPluginFactory exiting %s\n", msg);
    g_unixService->JD_ExitMonitor(factory_monitor);
}


JD_IMETHODIMP JavaPluginFactory5::ShowJavaConsole(void) {

    JavaVM5 *vm = GetJavaVM();

    if (!is_java_vm_started) return JD_ERROR_FAILURE;
    
    CWriteBuffer wb;
    trace("JavaPluginFactory5:ShowJavaConsole");
    wb.putInt(JAVA_PLUGIN_CONSOLE_SHOW);
    SendRequest(wb, FALSE);

    UNUSED(vm);

    return JD_OK;
}

/* Returns JVMManager */
IJVMManager* 
JavaPluginFactory5::GetJVMManager(void) {
        if (jvm_manager == NULL) {
            if (JD_FAILED(service_provider->QueryService(jCJVMManagerCID, 
						         jIJVMManagerIID, 
						         (ISupports **) &jvm_manager)))
                plugin_error("Could not get the JVM manager");

        }
	return jvm_manager;
}

/* The interface to CookieSupport to get the Cookie 
   Service from browser
*/
ICookieStorage* 
JavaPluginFactory5::GetCookieStorage(void) {
        if (cookieStorage == NULL) {
            if (JD_FAILED(service_provider->QueryService(jCPluginManagerCID,
    						 jICookieStorageIID,
    						 (ISupports **) &cookieStorage)))
                plugin_error("Could not get the CookieStorage");
        }
	return cookieStorage;
}
