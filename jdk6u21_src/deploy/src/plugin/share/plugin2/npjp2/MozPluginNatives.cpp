/*
 * @(#)MozPluginNatives.cpp	1.6 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "StdAfx.h"

#include <string.h>
#include "LocalFramePusher.h"
#include "MozPluginInstance.h"
#include "MozExports.h"
#include "AbstractPlugin.h"
#include "JavaObject.h"
#include "sun_plugin2_main_server_MozillaPlugin.h"
#include "sun_plugin2_main_server_MozillaBrowserService.h"

// Remove the following nsI*.h once the XPCOM dependencies are completely gone.
// In the interim, make the obsolete Mozilla headers think we are never compiling on the Mac
#undef XP_MACOSX
#include "nsISupports.h"
#include "nsIServiceManager.h"
#include "nsIPluginManager.h"
#include "nsIPluginManager2.h"
#include "nsICookieStorage.h"
#include "nsIJVMAuthTools.h"

// Ideally, we should place gServiceManager to the MozPluginExports.cpp
// so that once NP_Shutdown is called, it can be released if it has been
// instantiated. Since the headless VM inside the browser process won't
// go away anyways, we just hang this guy around until the browser shuts 
// down. Another advantage of putting this in here is that it will be
// easier to clean up just one file in future once the NPAPI adds the proxy
// & cookie APIs.
static nsIServiceManager *gServiceManager = NULL;
static const PRUint32 COOKIE_BUFFER_LEN = 8192;

// Contract ID of the legacy XPCOM plugin.
static const nsCID kPluginManagerCID = NS_PLUGINMANAGER_CID;
// Contract and interface IDs associated with the browser authenticator
static NS_DEFINE_CID(kIJVMAuthToolsCID, NS_JVMAUTHTOOLS_CID);
static NS_DEFINE_IID(kIJVMAuthToolsIID, NS_IJVMAUTHTOOLS_IID);
   
// Quick indication of whether NPN_GetValueForURL, NPN_SetValueForURL
// and NPN_GetAuthenticationInfo are available
extern bool g_haveCookieAndProxyNPAPIs;

void GetPluginService(const nsCID &contractIID, const nsIID &serviceIID, void **pPluginService) {

    if (gServiceManager == NULL)
        return;
    
    *pPluginService = NULL;
    gServiceManager->GetService(contractIID, serviceIID, pPluginService);
}
    
//----------------------------------------------------------------------
// Implementation of native methods on MozillaPlugin
//

extern "C" {

static void pluginThreadCallback(void* userData) {
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    // Guard against crashes in product builds
    if (env == NULL || MozPluginInstance::InitFailed()) {
        return;
    }
    jobject gRunnable = (jobject) userData;
    // Create a local reference to this and unreference the global one
    jobject lRunnable = env->NewLocalRef(gRunnable);
    env->DeleteGlobalRef(gRunnable);
    AbstractPlugin::runRunnable(lRunnable);
}

} // extern "C"

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    initServiceManager
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_initServiceManager(JNIEnv *env,
                                                              jclass unused) {
    // Get the global service manager which is the entry point (backdoor) for 
    // various services such as getting proxy/cookie and setting cookie etc.
    // In the foreseeable future (Firefox 3.x) we should ammend NPAPI to add
    // these services so that we can completely get out of the dependencies 
    // to XPCOM plugin  which is supposed to be used only by Mozilla internally.
    nsISupports* sm = NULL;
    if (MozNPN_GetValue(NULL, NPNVserviceManager, (void*) &sm) != NPERR_NO_ERROR || sm == NULL) {
        return;
    }    
    sm->QueryInterface(NS_GET_IID(nsIServiceManager), (void**)&gServiceManager);

    if (gServiceManager != NULL) {
        gServiceManager->AddRef();
    }

    NS_RELEASE(sm);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    invokeLater0
 * Signature: (JLjava/lang/Runnable;)V
 */
/*
 * This method provides the mechanism of asking browser main
 * thread to callback method encapsulating inside "runnable"
 * object.
 */
JNIEXPORT void JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_invokeLater0(JNIEnv *env,
                                                        jobject unused,
                                                        jlong npp,
                                                        jobject runnable) {
    if (npp == 0)
        return;

    jobject globalRef = env->NewGlobalRef(runnable);
    MozNPN_PluginThreadAsyncCall((NPP) npp, &pluginThreadCallback, (void*) globalRef);
}


/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    showDocument0
 * Signature: (JLjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_showDocument0(JNIEnv *env, 
                                                         jobject unused, 
                                                         jlong npp, 
                                                         jstring url, 
                                                         jstring target) {
    const char* urlChars = env->GetStringUTFChars(url, NULL);
    const char* targetChars = env->GetStringUTFChars(target, NULL);

    bool enabledPopups = false;
    if (target != NULL) {
        MozNPN_PushPopupsEnabledState((NPP)npp, true);
        enabledPopups = true;
    }

    MozNPN_GetURL((NPP) npp, urlChars, targetChars);

    if (enabledPopups) {
        MozNPN_PopPopupsEnabledState((NPP)npp);
    }

    env->ReleaseStringUTFChars(url, urlChars);
    env->ReleaseStringUTFChars(target, targetChars);

}        

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    showStatus0
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_showStatus0(JNIEnv *env, 
                                                       jobject unused, 
                                                       jlong npp, 
                                                       jstring status ) {
    const char* statusChars = env->GetStringUTFChars(status, NULL);

    MozNPN_Status((NPP) npp,statusChars);

    env->ReleaseStringUTFChars(status, statusChars);
}        

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    javaScriptGetWindow0
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_javaScriptGetWindow0(JNIEnv *env,
                                                                jobject unused,
                                                                jlong npp) {
    NPObject* window = NULL;
    NPError err = MozNPN_GetValue((NPP) npp, NPNVWindowNPObject, &window);
    if (err == NPERR_NO_ERROR)
        return (jlong) window;
    return 0;
}        

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnGetStringIdentifier
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnGetStringIdentifier(JNIEnv *env,
                                                                  jclass unused,
                                                                  jstring name) {
    const char* nameChars = env->GetStringUTFChars(name, NULL);
    jlong res = (jlong) MozNPN_GetStringIdentifier(nameChars);
    env->ReleaseStringUTFChars(name, nameChars);
    return res;
}        

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnGetIntIdentifier
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnGetIntIdentifier(JNIEnv *env,
                                                               jclass unused,
                                                               jint intid) {
    return (jlong) MozNPN_GetIntIdentifier(intid);
}        

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnIdentifierIsString
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnIdentifierIsString(JNIEnv *env,
                                                                 jclass unused,
                                                                 jlong npIdentifier) {
    return MozNPN_IdentifierIsString((NPIdentifier) npIdentifier);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnUTF8FromIdentifier
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnUTF8FromIdentifier(JNIEnv *env,
                                                                 jclass unused,
                                                                 jlong npIdentifier) {
    NPUTF8* name = MozNPN_UTF8FromIdentifier((NPIdentifier) npIdentifier);
    if (name == NULL)
        return NULL;
    jstring res = env->NewStringUTF(name);
    MozNPN_MemFree(name);
    return res;
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnIntFromIdentifier
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnIntFromIdentifier(JNIEnv *env,
                                                                jclass unused,
                                                                jlong npIdentifier) {
    return MozNPN_IntFromIdentifier((NPIdentifier) npIdentifier);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnRetainObject
 * Signature: (J)V
 */
JNIEXPORT void JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnRetainObject(JNIEnv *env,
                                                           jclass unused,
                                                           jlong npObjectPointer) {
    MozNPN_RetainObject((NPObject*) npObjectPointer);
}        

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnReleaseObject
 * Signature: (J)V
 */
JNIEXPORT void JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnReleaseObject(JNIEnv *env,
                                                            jclass unused,
                                                            jlong npObjectPointer) {
    MozNPN_ReleaseObject((NPObject*) npObjectPointer);
}        

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnInvoke
 * Signature: (JJJJIJ)Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnInvoke(JNIEnv *env,
                                                     jclass unused,
                                                     jlong npp,
                                                     jlong npObjectPointer,
                                                     jlong npIdentifier,
                                                     jlong argArrayPointer,
                                                     jint  numArgs,
                                                     jlong variantResPointer) {
    return MozNPN_Invoke((NPP) npp,
                      (NPObject*) npObjectPointer,
                      (NPIdentifier) npIdentifier,
                      (NPVariant*) argArrayPointer,
                      numArgs,
                      (NPVariant*) variantResPointer);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnEvaluate
 * Signature: (JJLjava/lang/String;J)Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnEvaluate(JNIEnv *env,
                                                       jclass unused,
                                                       jlong npp,
                                                       jlong npObjectPointer,
                                                       jstring code,
                                                       jlong variantResPointer) {
    NPString codeString;
    codeString.UTF8Characters = env->GetStringUTFChars(code, NULL);
    codeString.UTF8Length     = env->GetStringUTFLength(code);
    bool res = MozNPN_Evaluate((NPP) npp,
                            (NPObject*) npObjectPointer,
                            &codeString,
                            (NPVariant*) variantResPointer);
    env->ReleaseStringUTFChars(code, codeString.UTF8Characters);
    return res;
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnGetProperty
 * Signature: (JJJJ)Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnGetProperty(JNIEnv *env,
                                                          jclass unused,
                                                          jlong npp,
                                                          jlong npObjectPointer,
                                                          jlong npIdentifier,
                                                          jlong variantResPointer) {
    return MozNPN_GetProperty((NPP) npp,
                           (NPObject*) npObjectPointer,
                           (NPIdentifier) npIdentifier,
                           (NPVariant*) variantResPointer);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnSetProperty
 * Signature: (JJJJ)Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnSetProperty(JNIEnv *env,
                                                          jclass unused,
                                                          jlong npp,
                                                          jlong npObjectPointer,
                                                          jlong npIdentifier,
                                                          jlong variantArgPointer) {
    return MozNPN_SetProperty((NPP) npp,
                           (NPObject*) npObjectPointer,
                           (NPIdentifier) npIdentifier,
                           (NPVariant*) variantArgPointer);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnRemoveProperty
 * Signature: (JJJ)Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnRemoveProperty(JNIEnv *env,
                                                             jclass unused,
                                                             jlong npp,
                                                             jlong npObjectPointer,
                                                             jlong npIdentifier) {
    return MozNPN_RemoveProperty((NPP) npp,
                              (NPObject*) npObjectPointer,
                              (NPIdentifier) npIdentifier);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnHasProperty
 * Signature: (JJJ)Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnHasProperty(JNIEnv *env,
                                                          jclass unused,
                                                          jlong npp,
                                                          jlong npObjectPointer,
                                                          jlong npIdentifier) {
    return MozNPN_HasProperty((NPP) npp,
                           (NPObject*) npObjectPointer,
                           (NPIdentifier) npIdentifier);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnHasMethod
 * Signature: (JJJ)Z
 */
JNIEXPORT jboolean JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnHasMethod(JNIEnv *env,
                                                        jclass unused,
                                                        jlong npp,
                                                        jlong npObjectPointer,
                                                        jlong npIdentifier) {
    return MozNPN_HasMethod((NPP) npp,
                         (NPObject*) npObjectPointer,
                         (NPIdentifier) npIdentifier);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    npnSetException
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_npnSetException(JNIEnv *env,
                                                           jclass unused,
                                                           jlong npObjectPointer,
                                                           jstring message) {
    const char* messageChars = env->GetStringUTFChars(message, NULL);
    MozNPN_SetException((NPObject*) npObjectPointer, messageChars);
    env->ReleaseStringUTFChars(message, messageChars);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    allocateVariantArray
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_sun_plugin2_main_server_MozillaPlugin_allocateVariantArray
(JNIEnv *env, jobject unused, jint length)
{
    if (length == 0)
        return 0;

    NPVariant* args = new NPVariant[length];
    for (int i = 0; i < length; i++) {
        VOID_TO_NPVARIANT(args[i]);
    }
    return (jlong) args;
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    freeVariantArray
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_freeVariantArray
(JNIEnv *env, jobject unused, jlong arrayPointer, jint length)
{
    if (arrayPointer == 0)
        return;

    NPVariant* array = (NPVariant*) arrayPointer;
    for (int i = 0; i < length; i++) {
        MozNPN_ReleaseVariantValue(&array[i]);
    }
    delete[] array;
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElement0
 * Signature: (JIZ)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElement0__JIZ
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index, jboolean value)
{
    NPVariant* array = (NPVariant*) arrayPointer;
    BOOLEAN_TO_NPVARIANT(value, array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElement0
 * Signature: (JIB)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElement0__JIB
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index, jbyte value)
{
    NPVariant* array = (NPVariant*) arrayPointer;
    INT32_TO_NPVARIANT(value, array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElement0
 * Signature: (JIC)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElement0__JIC
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index, jchar value)
{
    NPVariant* array = (NPVariant*) arrayPointer;
    INT32_TO_NPVARIANT(value, array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElement0
 * Signature: (JIS)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElement0__JIS
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index, jshort value)
{
    NPVariant* array = (NPVariant*) arrayPointer;
    INT32_TO_NPVARIANT(value, array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElement0
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElement0__JII
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index, jint value)
{
    NPVariant* array = (NPVariant*) arrayPointer;
    INT32_TO_NPVARIANT(value, array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElement0
 * Signature: (JIJ)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElement0__JIJ
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index, jlong value)
{
    // FIXME: a double is the only data type supported by NPRuntime
    // that can represent (most of) the values a Java long can take
    // on. Should we consider converting a Java long to an
    // NPVariantType_Int32 if it's representable as an int?
    NPVariant* array = (NPVariant*) arrayPointer;
    DOUBLE_TO_NPVARIANT((double) value, array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElement0
 * Signature: (JIF)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElement0__JIF
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index, jfloat value)
{
    NPVariant* array = (NPVariant*) arrayPointer;
    DOUBLE_TO_NPVARIANT(value, array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElement0
 * Signature: (JID)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElement0__JID
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index, jdouble value)
{
    NPVariant* array = (NPVariant*) arrayPointer;
    DOUBLE_TO_NPVARIANT(value, array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElement0
 * Signature: (JILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElement0__JILjava_lang_String_2
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index, jstring value)
{
    NPVariant* array = (NPVariant*) arrayPointer;
    const char* chars = env->GetStringUTFChars(value, NULL);
    // Reallocate this storage using NPN_MemAlloc since releasing the
    // NPVariant will use NPN_MemFree on it
    int len = strlen(chars);
    char* memAllocedChars = (char*) MozNPN_MemAlloc((len + 1) * sizeof(char));
    strcpy(memAllocedChars, chars);
    env->ReleaseStringUTFChars(value, chars);
    STRINGN_TO_NPVARIANT(memAllocedChars, len, array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElementToScriptingObject0
 * Signature: (JIJ)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElementToScriptingObject0
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index, jlong value)
{
    NPVariant* array = (NPVariant*) arrayPointer;
    NPObject* obj = (NPObject*) value;
    if (obj == NULL) {
        NULL_TO_NPVARIANT(array[index]);
    } else {
        // NOTE: NPN_ReleaseVariantValue of an NPVariantType_Object
        // variant calls NPN_ReleaseObject, so we need to manually
        // increment the reference count of this object
        MozNPN_RetainObject(obj);
        OBJECT_TO_NPVARIANT(obj, array[index]);
    }
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setVariantArrayElementToVoid0
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setVariantArrayElementToVoid0
(JNIEnv *env, jobject unused, jlong arrayPointer, jint index)
{
    NPVariant* array = (NPVariant*) arrayPointer;
    VOID_TO_NPVARIANT(array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    variantArrayElementToObject0
 * Signature: (JJI)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_sun_plugin2_main_server_MozillaPlugin_variantArrayElementToObject0
(JNIEnv *env, jclass unused, jlong mozPluginInstance, jlong arrayPointer, jint index)
{
    MozPluginInstance* plugin = (MozPluginInstance*) mozPluginInstance;
    NPVariant* array = (NPVariant*) arrayPointer;
    return plugin->variantToJObject(env, &array[index]);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    allocateNPObject
 * Signature: (JLsun/plugin2/liveconnect/RemoteJavaObject;)J
 */
JNIEXPORT jlong JNICALL Java_sun_plugin2_main_server_MozillaPlugin_allocateNPObject
(JNIEnv *env, jclass unused, jlong mozPluginInstance, jobject object)
{
    if (mozPluginInstance == 0)
        return 0;
    MozPluginInstance* plugin = (MozPluginInstance*) mozPluginInstance;
    object = env->NewGlobalRef(object);
    JavaObject* obj = JavaObject::allocate(plugin->GetNPP(), object);
    return (jlong) obj;
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    allocateNPObjectForJavaNameSpace
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_sun_plugin2_main_server_MozillaPlugin_allocateNPObjectForJavaNameSpace
(JNIEnv *env, jclass unused, jlong mozPluginInstance, jstring javaNameSpace)
{
    if (mozPluginInstance == 0)
        return 0;
    MozPluginInstance* plugin = (MozPluginInstance*) mozPluginInstance;
    const char* nameSpaceChars = env->GetStringUTFChars(javaNameSpace, NULL);
    if (nameSpaceChars == NULL)
        return 0;
    JavaObject* obj = JavaObject::allocateForJavaNameSpace(plugin->GetNPP(), nameSpaceChars);
    env->ReleaseStringUTFChars(javaNameSpace, nameSpaceChars);
    return (jlong) obj;
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    getProxy0
 * Signature: (JLjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_plugin2_main_server_MozillaPlugin_getProxy0
(JNIEnv *env, jobject unused, jlong npp, jstring urlString) {

    jstring proxyString = NULL;
                                                
    if (g_haveCookieAndProxyNPAPIs) {
        // Prefer this implementation but leave the old one around as a fallback for now
        const char* url = env->GetStringUTFChars(urlString, NULL);
        char* value = NULL;
        uint32_t len = 0;
        NPError res = MozNPN_GetValueForURL((NPP) npp,
                                            NPNURLVProxy,
                                            url,
                                            &value,
                                            &len);
        env->ReleaseStringUTFChars(urlString, url);
        if (res == NPERR_NO_ERROR) {
            if (value != NULL) {
                // NOTE: ignoring len
                proxyString = env->NewStringUTF(value);
                MozNPN_MemFree(value);
                return proxyString;
            }
        }
    }

    nsIPluginManager2 *pluginMgr = NULL;    

    GetPluginService(kPluginManagerCID, NS_GET_IID(nsIPluginManager2), (void**)&pluginMgr);

    if (pluginMgr != NULL) {
        const char* url = env->GetStringUTFChars(urlString, NULL);
        assert(url != NULL);
        char* proxyInfo;
        if (pluginMgr->FindProxyForURL(url, &proxyInfo) == NS_OK &&
            proxyInfo != NULL) {
            int proxyInfoLen = strlen(proxyInfo);
            char* proxies = (char*)MozNPN_MemAlloc((proxyInfoLen + 1)* sizeof(char));
            if (proxies != NULL) {
                strncpy(proxies, proxyInfo, proxyInfoLen);
                proxies[proxyInfoLen] = '\0';
                proxyString = env->NewStringUTF(proxies);
              
                MozNPN_MemFree(proxies);
            }
        }
        pluginMgr->Release();
        env->ReleaseStringUTFChars(urlString, url);
    }
    return proxyString;
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    setCookie0
 * Signature: (JLjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_sun_plugin2_main_server_MozillaPlugin_setCookie0
(JNIEnv *env, jobject unused, jlong npp, jstring urlString, jstring value) {
    if (value == NULL) return; // nothing to set.
    
    if (g_haveCookieAndProxyNPAPIs) {
        // Prefer this implementation but leave the old one around as a fallback for now
        const char* url = env->GetStringUTFChars(urlString, NULL);
        const char* inCookie = env->GetStringUTFChars(value, NULL);
        // FIXME: should perhaps have different way of passing in this
        // information to handle the case of embedded nulls
        NPError res = MozNPN_SetValueForURL((NPP) npp,
                                            NPNURLVCookie,
                                            url,
                                            inCookie,
                                            strlen(inCookie));
        env->ReleaseStringUTFChars(urlString, url);
        env->ReleaseStringUTFChars(value, inCookie);
        if (res != NPERR_NO_ERROR) {
            char msgBuf[512];
            sprintf(msgBuf, "Error %d while setting cookie", res);
            env->ThrowNew(env->FindClass("java/lang/RuntimeException"),
                          msgBuf);
        }
        return;
    }

    nsICookieStorage *pCookieStorage = NULL;
    GetPluginService(kPluginManagerCID, NS_GET_IID(nsICookieStorage), (void**)&pCookieStorage);

    if (pCookieStorage != NULL) {
        const char* url = env->GetStringUTFChars(urlString, NULL);
        const char* inCookie = env->GetStringUTFChars(value, NULL);

        assert(url != NULL && inCookie != NULL);
        pCookieStorage->SetCookie(url, inCookie, strlen(inCookie));

        env->ReleaseStringUTFChars(urlString, url);
        env->ReleaseStringUTFChars(value, inCookie);
        
        pCookieStorage->Release();
    }
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    getCookie0
 * Signature: (JLjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_plugin2_main_server_MozillaPlugin_getCookie0
(JNIEnv *env, jobject unused, jlong npp, jstring urlString) {

    if (urlString == NULL) return NULL; // nothing to set.
    
    jstring cookieString = NULL; 

    if (g_haveCookieAndProxyNPAPIs) {
        // Prefer this implementation but leave the old one around as a fallback for now
        const char* url = env->GetStringUTFChars(urlString, NULL);
        char* value = NULL;
        uint32_t len = 0;
        NPError res = MozNPN_GetValueForURL((NPP) npp,
                                            NPNURLVCookie,
                                            url,
                                            &value,
                                            &len);
        env->ReleaseStringUTFChars(urlString, url);
        if (res == NPERR_NO_ERROR) {
            if (value != NULL) {
                // NOTE: ignoring len
                cookieString = env->NewStringUTF(value);
                MozNPN_MemFree(value);
                return cookieString;
            }
        }
    }

    nsICookieStorage *pCookieStorage = NULL;
    GetPluginService(kPluginManagerCID, NS_GET_IID(nsICookieStorage), (void**)&pCookieStorage);
 
    if (pCookieStorage != NULL) {
        const char* url = env->GetStringUTFChars(urlString, NULL);
        assert(url != NULL);
        PRUint32 cookieBufferLen = COOKIE_BUFFER_LEN;
        char* cookieBuffer = (char*)MozNPN_MemAlloc(cookieBufferLen * sizeof(char));
        
        if (cookieBuffer != NULL) {
            if (pCookieStorage->GetCookie(url, (void*)cookieBuffer, cookieBufferLen) == NS_OK) {
                cookieString = env->NewStringUTF(cookieBuffer);
            }
            MozNPN_MemFree(cookieBuffer);
        }
        env->ReleaseStringUTFChars(urlString, url);
        
        pCookieStorage->Release();
    }

    return cookieString;
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    getAuthentication0
 * Signature: (JLjava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;)[C
 */
JNIEXPORT jcharArray JNICALL Java_sun_plugin2_main_server_MozillaPlugin_getAuthentication0
(JNIEnv *env, jobject unused, jlong npp, jstring protocol, jstring host, jint port, jstring scheme, jstring realm) {

    if (g_haveCookieAndProxyNPAPIs) {
        // Prefer this implementation but leave the old one around as a fallback for now
        const char* lpszProtocol = env->GetStringUTFChars(protocol, (jboolean*)0);
        const char* lpszHost = env->GetStringUTFChars(host, (jboolean*)0);
        const char* lpszScheme = env->GetStringUTFChars(scheme, (jboolean*)0);
        const char* lpszRealm = env->GetStringUTFChars(realm, (jboolean*)0);
        char* username = NULL;
        uint32_t ulen = 0;
        char* password = NULL;
        uint32_t plen = 0;

        NPError res = MozNPN_GetAuthenticationInfo((NPP) npp,
                                                   lpszProtocol,
                                                   lpszHost,
                                                   port,
                                                   lpszScheme,
                                                   lpszRealm,
                                                   &username,
                                                   &ulen,
                                                   &password,
                                                   &plen);
        env->ReleaseStringUTFChars(protocol, lpszProtocol);
        env->ReleaseStringUTFChars(host, lpszHost);
        env->ReleaseStringUTFChars(scheme, lpszScheme);
        env->ReleaseStringUTFChars(realm, lpszRealm);
        if (res != NPERR_NO_ERROR) {
            return NULL;
        }

        if (username == NULL || password == NULL) {
            return NULL;
        }

        int len = strlen(username) + strlen(password);
        jcharArray retJCA = MozPluginInstance::pdAuthInfoToCharArray(env, len,
                                                                     username,
                                                                     password);
        MozNPN_MemFree(username);
        MozNPN_MemFree(password);
        return retJCA;
    }

    nsIJVMAuthTools *pJVMAuthTools = NULL;
    GetPluginService(kIJVMAuthToolsCID, kIJVMAuthToolsIID, (void**)&pJVMAuthTools);

    if (pJVMAuthTools == NULL) {
	return NULL;
    }

    const char* lpszProtocol = env->GetStringUTFChars(protocol, (jboolean*)0);
    const char* lpszHost = env->GetStringUTFChars(host, (jboolean*)0);
    const char* lpszScheme = env->GetStringUTFChars(scheme, (jboolean*)0);
    const char* lpszRealm = env->GetStringUTFChars(realm, (jboolean*)0);

    nsIAuthenticationInfo* pAuthInfo;

    unsigned int res = pJVMAuthTools->GetAuthenticationInfo(lpszProtocol, lpszHost, port, lpszScheme, lpszRealm, &pAuthInfo);

    env->ReleaseStringUTFChars(protocol, lpszProtocol);
    env->ReleaseStringUTFChars(host, lpszHost);
    env->ReleaseStringUTFChars(scheme, lpszScheme);
    env->ReleaseStringUTFChars(realm, lpszRealm);
    if(NS_FAILED(res)) {
	return NULL;
    }

    char* lpszName = NULL;
    char* lpszPasswd = NULL;
    if(NS_FAILED(pAuthInfo->GetUsername((const char**)&lpszName)) 
	|| NS_FAILED(pAuthInfo->GetPassword((const char**)&lpszPasswd))) {
	pAuthInfo->Release();
	return NULL;
    }

    int len = strlen(lpszName) + strlen(lpszPasswd);
    jcharArray retJCA = NULL;

    if(0 != len) {
	retJCA = MozPluginInstance::pdAuthInfoToCharArray(env, len, (const char*)lpszName, (const char*)lpszPasswd);
    }

    pAuthInfo->Release();

    return retJCA;

}

#ifdef macosx
#include <Carbon/Carbon.h>
#include <HIToolbox/MacWindows.h>

extern "C" {
  typedef int      CGSWindow;
  extern CGSWindow GetNativeWindowFromWindowRef(WindowRef);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    getNativeWindowFromCarbonWindowRef
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_getNativeWindowFromCarbonWindowRef(JNIEnv *env, 
                                                                              jobject unused,
                                                                              jlong windowRef) {
    return GetNativeWindowFromWindowRef((WindowRef) windowRef);
}

/*
 * Class:     sun_plugin2_main_server_MozillaPlugin
 * Method:    getCarbonWindowBounds
 * Signature: (JI[I)V
 */
#if defined(__i386) || defined(__PPC)
JNIEXPORT void JNICALL 
Java_sun_plugin2_main_server_MozillaPlugin_getCarbonWindowBounds(JNIEnv *env, 
                                                                 jobject unused,
                                                                 jlong windowRef,
                                                                 jint which,
                                                                 jintArray bounds) {
    WindowRef window = (WindowRef) windowRef;
    Rect rect;
    GetWindowBounds(window, (WindowRegionCode) which, &rect);
    jint values[4];
    values[0] = rect.top;
    values[1] = rect.left;
    values[2] = rect.bottom;
    values[3] = rect.right;
    env->SetIntArrayRegion(bounds, 0, 4, values);
}
#endif
#endif
