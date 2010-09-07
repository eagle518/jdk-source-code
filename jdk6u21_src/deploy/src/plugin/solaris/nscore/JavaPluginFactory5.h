/*
 * @(#)JavaPluginFactory5.h	1.33 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JAVAPLUGINFACTORY5_H
#define JAVAPLUGINFACTORY5_H

#include "commonhdr.h"
#include "jni.h"
#include "IJVMPlugin.h"
#include "IPlugin.h"
#include "IPluginServiceProvider.h"
#include "IPluginManager.h"
#include "IJVMManager.h"
#include "IJVMConsole.h"
#include "IUnixService.h"
#include "ICookieStorage.h"

#include "CookieSupport.h"
#include "CWriteBuffer.h"
#include "JavaPluginInstance5.h"
#include "ProxySupport5.h"
#include "remotejni.h"
#include "JavaVM5.h"

struct JVMInitArgs {
       JDUint32    version;
       const char* classpathAdditions;     // appended to the JVM's classpath
// other fields may be added here for version numbers beyond 0x00010000
};

/**
* nsJVMInitArgs_Version is the current version number for the nsJVMInitArgs
* struct specified above. The nsVersionOk macro should be used when comparing
* a supplied nsJVMInitArgs struct against this version.
*/
#define JVMInitArgs_Version   0x00010000L

/* Fill in the function pointer table with the entry points for the
 * 5.0 version
 */

/* Implementation of the 5.0 functionality required for the JavaPlugin.
 *  Basically, a class version of the PluginFactoryFunction table
 *  defined in JavaPluginFactory.h 
 */
class JavaPluginFactory5 : public IJVMPlugin, IPlugin {
public:
  
    JD_DECL_ISUPPORTS

    /* Constructor/destructor */    
    JavaPluginFactory5(IPluginServiceProvider* pProvider);

    /* Destructor releases the manager and shutsdown the vm and deletes
     * the related structures. */
    virtual ~JavaPluginFactory5(void);

    //===============================================================
    // IFactory
    //===============================================================
    /* Creates a nsPluginInstance. Since we don't know how we will get
     * called, we have to potentially start the VM if it is not running. */
    JD_IMETHOD CreateInstance(ISupports *aOuter, const JDIID & iid,
			      void **result);
    
    JD_IMETHOD LockFactory(JDBool aLock);

    JD_IMETHOD CreatePluginInstance(ISupports *aOuter, JDREFNSIID aIID,
                                    const char* aPluginMIMEType,
                                    void **aResult);

    //===============================================================
    //  IPlugin
    //===============================================================
    JD_IMETHOD Initialize();
    
    JD_IMETHOD Shutdown(void);

    // (Corresponds to NPP_GetMIMEDescription.)
    JD_IMETHOD GetMIMEDescription(const char **resDesc);
    
    // (Corresponds to NPP_GetValue.)
    JD_IMETHOD GetValue(JDPluginVariable variable, void *value);

    //===============================================================
    // IJVMPlugin
    //===============================================================
    JD_IMETHOD GetClassPath(const char* *result);

    /* 
     * GetJavaWrapper provides a java wrapper for (C/C++) browser object
     * pointers.  Map between browser objects (browser_obj - C/C++
     * pointers) and Java objects, (java_obj - a handle to a
     * "JSObject"). Get a Java wrapper for a browser object, so that the
     * browser object can be handled within java code and then passed back
     * to the browser during some JSObject call.  The java wrapper object
     * (a JSObject) has a pointer to the original browser object
     * (browser_obj)
     */
    JD_IMETHOD GetJavaWrapper(JNIEnv* jenv, jint jint, jobject* obj);

    JD_IMETHOD UnwrapJavaWrapper(JNIEnv* jenv, jobject jobj, jint* obj);
    
    JD_IMETHOD CreateSecureEnv(JNIEnv* proxy, ISecureEnv* *result);
    
    //===============================================================
    // All the following functions are plugin internal i.e. they are 
    // not called by the browser and are not part of the abstract 
    // browser interface to the plugin
    //===============================================================
    JD_IMETHOD StartupJVM(JVMInitArgs *initargs);

    JD_IMETHOD ShutdownJVM(JDBool fullShutdown = JD_FALSE);

    // Called by JavaPluginInstance5::Initialize
    JD_IMETHOD_(void) CreateApplet(const char *appletType, int appletNumber, 
				   int argc, char **argn, char **argv);

    /* These methods are called from CJavaConsole which we agregate 
       Called by CJavaConsole::Show
    */
    JD_IMETHOD ShowJavaConsole(void);

    /* Return the Proxy Support class, which handles interactions
     * with the browser to get the proxies, using GetURL */
    ProxySupport5* GetProxySupport(void);

    /* Send a request over the command pipe. */
    void SendRequest(const CWriteBuffer& wb, int wait_for_reply);

    /* Both the tables below should be split out into their own classes */
    /* Interface to a table of plugin instances */
    int RegisterInstance(JavaPluginInstance5 * pi);
   
     void UnregisterInstance(JavaPluginInstance5 * pi);
    /* Obtain an instance, given an index into the instance table.
     * An index of -1 results in a random instance being picked */
    JavaPluginInstance5* GetInstance(int index);

    /* Indicate that a particular instance index is currently in use and should not be
       reused yet. */
    void LockInstanceIndex(int index);

    /* Indicate that a particular instance index may be reused. */
    void UnlockInstanceIndex(int index);

    /* Interface to a table of remote envs. */
    int RegisterRemoteEnv(RemoteJNIEnv* env, JNIEnv* proxyenv);

    int UnregisterRemoteEnv(RemoteJNIEnv* env);

    RemoteJNIEnv* GetRemoteEnv(JNIEnv* proxy_env);

    /* Return the browser-side plugin manager */
    IPluginManager* GetPluginManager(void) {
        if (plugin_manager == NULL) 
	    fprintf(stderr, "Internal error: Null plugin manager");
	return plugin_manager;
    }
    
    IPluginServiceProvider* GetServiceProvider(void) {
        return service_provider;
    }

    IJVMManager*  GetJVMManager(void);

    /* The interface to CookieSupport to get the Cookie 
        Service from browser
    */
    ICookieStorage* GetCookieStorage(void);
    
    /*The interface to JavaVM5 to get CookieSupport
      returns a pointer to cookiesupport */
    CookieSupport *GetCookieSupport(void){
	if (cookieSupport == NULL)
	    fprintf(stderr, "Internal error: Null cookieSupport");
      return cookieSupport;
    }
    /* Enter/exit the monitor associated with this factory */
    /* Enter the monitor associated with this factory for use
     * in the associated classes, invoke.cpp and JavaVM5 */
    void EnterMonitor(const char* msg);

    /* Exit the monitor associated with this factory */
    void ExitMonitor(const char* msg);

    /* Get the java object associated with the plugin instance
       whose index is plugin_number */
    jobject GetJavaObjectForInstance(int plugin_number);

    /* Return the JavaVM object, making sure it is started if necc */
    JavaVM5* GetJavaVM(void);

    static JD_METHOD Create(ISupports* sm, 
                            const JDIID& aIID, 
                            void* *aInstancePtr);

private:

    /* The plugin manager provides us with our link to the browser */
    IPluginManager* plugin_manager;
    
    IJVMManager* jvm_manager;

    /* Indicates whether the Java VM has already been started or not */
    int is_java_vm_started;

    /* Pointer to cookieSupport class */
    CookieSupport *cookieSupport;
    
    /* where to obtain cookie */
    ICookieStorage *cookieStorage;

    /* Table of plugin instances */
    JavaPluginInstance5 **plugin_instances;

    /* List indicating that a particular plugin instance's index is still potentially
       in use by the java_vm process and should not be reused yet */
    bool* locked_plugin_instances;

    void*  factory_monitor;

    /* Table of remote envs */
    RemoteJNIEnv **current_envs;

    /* Table of corresponding proxy envs */
    JNIEnv **current_proxy_envs;

    /* Pointer to the JavaVM that is the proxy for the out of process VM */
    JavaVM5* javaVM;

    /* Pointer to proxy supporting structures */
    ProxySupport5 *proxy_support;
   
    /* Name of this plugin */
    char *pluginNameString;

    IPluginServiceProvider* service_provider;
    
    IJVMConsole * m_pIJVMConsole;

    bool isInitialized;
};

extern "C" {
JavaPluginFactory5* get_global_factory(void);

IUnixService*  get_global_us(void);

JD_METHOD createPluginFactory(ISupports* sm, 
			      IUnixService* us,
			      IFactory* *res);
}

#endif





