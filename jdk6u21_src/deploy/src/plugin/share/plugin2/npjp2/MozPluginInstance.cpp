/*
 * @(#)MozPluginInstance.cpp	1.27 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "StdAfx.h"

#include <string.h>
#include "JavaVM.h"
#include "JNIExceptions.h"
#include "LocalFramePusher.h"
#include "MozPluginInstance.h"
#include "MozExports.h"
#include "AbstractPlugin.h"
#include "JavaObject.h"
#include "utils.h"

static jclass    stringClass = NULL;

static bool g_isInitialized = false;
static bool g_initFailed = true;
static bool g_launchjnlp = false;

jclass    MozPluginInstance::mozPluginClass              = NULL;
jmethodID MozPluginInstance::mozPluginCtorID             = NULL;
jmethodID MozPluginInstance::mozPluginAddParametersID    = NULL;
jmethodID MozPluginInstance::mozPluginSetWindowID        = NULL;
jmethodID MozPluginInstance::mozPluginDestroyID          = NULL;
jmethodID MozPluginInstance::mozPluginPrintID            = NULL;
jmethodID MozPluginInstance::mozPluginUpdateLocationAndClipID = NULL;

bool MozPluginInstance::InitFailed() {
    return g_initFailed;
}

MozPluginInstance::MozPluginInstance(NPP instance,
                                     const char* pluginMimeType,
                                     short argc, char* argn[], char* argv[]) {

    // Try load docbase and launchjnlp PARAM
    if (argc > 0) {
        char* docbase = NULL;
        char* jnlphref = NULL;
        for (int i = 0; i < argc; i++) {

            if (strcmp(argn[i], "launchjnlp") == 0) {
                jnlphref = argv[i];
            } else if (strcmp(argn[i], "docbase") == 0) {
                docbase = argv[i];
            }

            // If we have both PARAM specified, we are in Java Web Start mode
            if (jnlphref && docbase) {
                g_launchjnlp = true;
                m_pluginInstance = NULL;
                m_pluginObject = NULL;
                launchJNLP(jnlphref, docbase);
                return;
            }
        }
    }

    m_pluginInstance = instance;
    m_appletNPObject = NULL;

    // Try to get the docbase for the plugin instance.
    char* docbase = GetDocumentBase();

    if (Initialize() == false) {
        return;
    }

    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    assert(env != NULL);
    // Guard against crashes in product builds
    if (env == NULL || g_initFailed) {
        return;
    }
    jstring jdocbase = NULL;

    if (docbase != NULL) {
        jdocbase = env->NewStringUTF(docbase);
        delete [] docbase;
    }

    // convert native m_mimeType string to jstring
    jstring jmimeType = NULL;
    if (pluginMimeType != NULL) {
        jmimeType = env->NewStringUTF(pluginMimeType);
    }
  
    jobject plugin = env->NewObject(mozPluginClass, 
                                    mozPluginCtorID, 
                                    (jlong) this,
                                    (jlong) instance,
                                    jdocbase,
                                    jmimeType);
    CHECK_EXCEPTION(env);
    
    // Put the key and value pairs into the object array.
    jstring key;
    jstring val;

    jobjectArray keyArray = env->NewObjectArray(argc, stringClass, NULL);
    jobjectArray valArray = env->NewObjectArray(argc, stringClass, NULL);
  
    for (int i = 0; i < argc; i++) {
        key = env->NewStringUTF(argn[i]);
        val = env->NewStringUTF(argv[i]);
        env->SetObjectArrayElement(keyArray, i, key);
        env->SetObjectArrayElement(valArray, i, val);
    }
  
    env->CallVoidMethod(plugin, mozPluginAddParametersID, keyArray, valArray);
 
    assert(env->ExceptionOccurred() == NULL);
  
    m_pluginObject = env->NewGlobalRef(plugin);
    pdConstruct();
}

MozPluginInstance::~MozPluginInstance() {

    if (g_launchjnlp) {
        g_launchjnlp = false;
        return;
    }
  
    pdDelete();

    if (m_appletNPObject != NULL) {
        MozNPN_ReleaseObject(m_appletNPObject);
        m_appletNPObject = NULL;
    }

    if (m_pluginObject != NULL) {
        JNIEnv* env = JavaVM_GetJNIEnv();
        // Guard against crashes in product builds
        if (env != NULL) {
            env->DeleteGlobalRef(m_pluginObject);
            m_pluginObject = NULL;
        }
    }

    // Note that we do NOT currently detach the current thread from
    // the JVM upon plugin termination. The Java-side code for the
    // MozillaPlugin assumes that the browser main thread has a stable
    // java.lang.Thread identity for maintenance of its runnable
    // queues and to give the system the ability to service Plugin
    // invokeLater requests coming from other plugin instances (for
    // example, other applets on the same web page). When a thread is
    // detached from the JVM and re-attached, it becomes a different
    // java.lang.Thread instance, which breaks the logic in the
    // MozillaPlugin. More thought is needed in this area, in
    // particular around the maintenance of the runnable queues and
    // their scope. The problem is exacerbated by Firefox's
    // asynchronous disposal of plugin instances, but the basic
    // problem is that the definition of the browser main thread
    // changes over time. Regardless, because Firefox is currently
    // completely single-threaded, leaving the browser main thread
    // attached to the JVM will not introduce any resource leaks.
    //
    // A more complete fix would involve maintaining a count at the
    // Java level of applets currently running on this browser main
    // thread. The JVM could then be detached when this count goes to
    // zero. At that point Java-level references to that Thread
    // instance would need to be removed from the various HashMaps.
#if 0
    JavaVM_DetachCurrentThread();
#endif
}
bool MozPluginInstance::GlobalInitialize() {
    return true;
}
//
// Initialize is mostly performing the VM starting up and etc.
// Returns true if no error occurs, otherwise, returns false.
//
bool MozPluginInstance::Initialize() {

    if (g_isInitialized) {
        return true;
    }

    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    assert(env != NULL);
    // Guard against crashes in product builds
    if (env == NULL) { 
        return false;
    }

    // helper JNI globals
    stringClass    = (jclass)env->NewGlobalRef(env->FindClass("java/lang/String"));
    
    mozPluginClass = env->FindClass("sun/plugin2/main/server/MozillaPlugin");
    if (mozPluginClass == NULL) {
        CLEAR_EXCEPTION(env);
        return false;
    }

    mozPluginClass           = (jclass)env->NewGlobalRef(mozPluginClass);
    mozPluginCtorID          = env->GetMethodID(mozPluginClass, "<init>", "(JJLjava/lang/String;Ljava/lang/String;)V");
    mozPluginAddParametersID = env->GetMethodID(mozPluginClass, "addParameters",
                                                "([Ljava/lang/String;[Ljava/lang/String;)V");
    mozPluginSetWindowID     = env->GetMethodID(mozPluginClass, "setWindow",
                                                "(JIIIIIIII)V");
    mozPluginDestroyID       = env->GetMethodID(mozPluginClass, "destroy", "()V");
    mozPluginPrintID         = env->GetMethodID(mozPluginClass, "print", "(JIIII)Z");
    mozPluginUpdateLocationAndClipID = env->GetMethodID(mozPluginClass, "updateLocationAndClip", "()V");
  
    CHECK_EXCEPTION_VAL(env, false);

    // Initialize JavaScript -> Java support
    if (!AbstractPlugin::initialize()) {
        return false;
    }

    JavaObject::initialize();

    g_isInitialized = true;
    g_initFailed = false;
    return true;
}

bool MozPluginInstance::SetWindow(void* window, 
                                  uint x, uint y, 
                                  uint width, uint height,
                                  uint clipTop,
                                  uint clipLeft,
                                  uint clipBottom,
                                  uint clipRight) {
  
    if (g_launchjnlp) {
        return false;
    }

    assert(m_pluginObject != NULL);
    if (m_pluginObject == NULL) {
        // Something went wrong during initialization
        return false;
    }
    
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    // Guard against crashes in product builds
    if (env == NULL || g_initFailed) { 
        return false;
    }
    // "window" could be NULL, in which case, call the java code
    // to release any related resources.
    if (window != NULL) {
        // NOTE: it is currently very important that we call
        // pdSetWindow before calling up to Java, since the Java code
        // might do operations that require platform-dependent setup
        // to have been done
        pdSetWindow(window, x, y, width, height, clipTop, clipLeft, clipBottom, clipRight);

        if (env->ExceptionOccurred() != NULL) {
            env->ExceptionClear();
        }
        env->CallVoidMethod(m_pluginObject,
                            mozPluginSetWindowID,
                            (jlong)window,
                            (jint)x, (jint)y, (jint)width, (jint)height,
                            (jint) clipTop, (jint) clipLeft, (jint) clipBottom, (jint) clipRight);
        
        CHECK_EXCEPTION_VAL(env, false);
    } else {
        env->CallVoidMethod(m_pluginObject,
                            mozPluginSetWindowID,
                            0L, 0, 0, 0, 0);
        
        // Do this after calling Java on the assumption that it might
        // also use invokeLater as per above
        pdSetWindow(window, x, y, width, height, clipTop, clipLeft, clipBottom, clipRight);
    }
    
    return true;
}

bool MozPluginInstance::Print(NPPrint* platformPrint) { 
    assert(m_pluginObject != NULL);
    if (m_pluginObject == NULL) {
        // Something went wrong during initialization
        return false;
    }

    if(platformPrint == NULL)   // trap invalid parameter
        return false;

    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    // Guard against crashes in product builds
    if (env == NULL || g_initFailed) { 
        return false;
    }
    if (env->ExceptionOccurred() != NULL) {
        env->ExceptionClear();
    }

    if (platformPrint->mode == NP_FULL) {
        // We will try, but will set platformPrint->print.fullPrint.pluginPrinted to the result
        // of this printing to let the browser know that:
        // - TRUE means plugin has successfully handled fullscreen printing.
        // - FALSE means fullscreen printing by plugin failed. Browser should take care of
        //   of it.

        bool printResult = (bool) env->CallBooleanMethod(m_pluginObject,
                                                         mozPluginPrintID,
                                                         (jlong) platformPrint->print.fullPrint.platformPrint);
        platformPrint->print.fullPrint.pluginPrinted = printResult; // Do the default
        return printResult;
    } else { // If not fullscreen, we must be embedded

        return pdPrintEmbedded(platformPrint->print.embedPrint);
    }
 
}


// This method is called when NPP_Destroy is called from Mozilla.
void MozPluginInstance::Destroy() {
    if (g_launchjnlp) {
        return;
    }

    assert(m_pluginObject != NULL);
    if (m_pluginObject == NULL) {
        // Something went wrong during initialization
        return;
    }

    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    // Guard against crashes in product builds
    if (env == NULL || g_initFailed) { 
        return;
    }
    env->CallVoidMethod(m_pluginObject, mozPluginDestroyID);
    CLEAR_EXCEPTION(env);
}

// This method retrieves the URL of the document base containing
// the java applet. 
// If it succeeds, it returns a newly-allocated, null-terminated
// string which must be freed later via delete[].
// If it fails, it returns NULL.
char* MozPluginInstance::GetDocumentBase() {

  // "res" will be the return result.
  char* res = NULL;

  NPObject *window_obj = NULL;
  if (MozNPN_GetValue(m_pluginInstance, NPNVWindowNPObject, 
                   &window_obj) != NPERR_NO_ERROR) {
    return res; // fail to grab the DOM window object.
  }

  // retrive "document.URL" from the DOM window is the way we get the 
  // codebase of the applet.
  NPIdentifier doc_id = MozNPN_GetStringIdentifier("document");
  NPVariant doc_var;
  if (doc_id == NULL ||
      !MozNPN_GetProperty(m_pluginInstance, window_obj, doc_id, &doc_var)) {
    return res;
  }
  // Convert doc_obj to NPObject so that we could use it to grab the URL variant.
  NPObject *doc_obj = NPVARIANT_TO_OBJECT(doc_var);
  NPIdentifier url_id = MozNPN_GetStringIdentifier("URL");
  if (url_id != NULL) {
    bool isString = false;
    NPVariant url_var;
    if (MozNPN_GetProperty(m_pluginInstance, doc_obj, url_id, &url_var)) {
      if (NPVARIANT_IS_STRING(url_var)) {
        isString = true;
      } else {
        // For certain types of hosting documents the URL isn't there
        // Try get documentURI instead
        MozNPN_ReleaseVariantValue(&url_var);
        NPIdentifier documentURI_id = MozNPN_GetStringIdentifier("documentURI");
        if (documentURI_id == NULL || 
            !MozNPN_GetProperty(m_pluginInstance, doc_obj, documentURI_id, &url_var)) {
          MozNPN_ReleaseVariantValue(&doc_var);
          return res;
        }
        isString = NPVARIANT_IS_STRING(url_var);
      }

      if (isString) {
        NPString url_string = NPVARIANT_TO_STRING(url_var);
        res = new char[url_string.UTF8Length + 1];
        strncpy(res, url_string.UTF8Characters, url_string.UTF8Length);
        res[url_string.UTF8Length] = '\0';
      }

      MozNPN_ReleaseVariantValue(&url_var);
    }
  } 
  
  MozNPN_ReleaseVariantValue(&doc_var); // This will release the doc_obj as well.
  return res;
}

jobject MozPluginInstance::variantToJObject(JNIEnv* env, NPVariant* variant) {
    if (NPVARIANT_IS_VOID(*variant) ||
        NPVARIANT_IS_NULL(*variant)) {
        return NULL;
    }

    if (NPVARIANT_IS_BOOLEAN(*variant)) {
        return AbstractPlugin::newBoolean(env, m_pluginObject, NPVARIANT_TO_BOOLEAN(*variant));
    }

    if (NPVARIANT_IS_INT32(*variant)) {
        return AbstractPlugin::newInteger(env, m_pluginObject, NPVARIANT_TO_INT32(*variant));
    }

    if (NPVARIANT_IS_DOUBLE(*variant)) {
        return AbstractPlugin::newDouble(env, m_pluginObject, NPVARIANT_TO_DOUBLE(*variant));
    }

    if (NPVARIANT_IS_STRING(*variant)) {
        // This allocation / free is unfortunate but necessary since
        // the UTF8Characters are not guaranteed to be null-terminated
        // (though they happen to be in Firefox)
        NPString str = NPVARIANT_TO_STRING(*variant);
        char* chars = new char[str.UTF8Length + 1];
        memcpy(chars, str.UTF8Characters, str.UTF8Length);
        chars[str.UTF8Length] = '\0';
        jobject res = env->NewStringUTF(chars);
        delete[] chars;
        CHECK_EXCEPTION_VAL(env, NULL);
        return res;
    }

    if (NPVARIANT_IS_OBJECT(*variant)) {
        NPObject* obj = NPVARIANT_TO_OBJECT(*variant);
        if (JavaObject::isJavaObject(obj)) {
            return ((JavaObject*) obj)->targetObject;
        }
        return AbstractPlugin::wrapOrUnwrapScriptingObject(env, m_pluginObject, (jlong) obj);
    }

    // Should not reach here
    return NULL;
}

jobject MozPluginInstance::getJavaMozillaPluginInstance() {
    return m_pluginObject;
}

NPObject* MozPluginInstance::getAppletNPObject() {
    assert(m_pluginObject != NULL);
    if (m_pluginObject == NULL) {
        // Something went wrong during initialization
        return NULL;
    }

    if (m_appletNPObject == NULL) {
        LocalFramePusher pusher;
        // Guard against crashes in product builds
        if (pusher.getEnv() == NULL || g_initFailed) { 
            return NULL;
        }
        m_appletNPObject =
            (NPObject*) AbstractPlugin::getScriptingObjectForApplet(m_pluginObject, 0);
        // This is the retain call for our persistent reference
        if (m_appletNPObject != NULL) {
            MozNPN_RetainObject(m_appletNPObject);
        }
    }

    // Always need to bump the reference count of the returned NPObject
    if (m_appletNPObject != NULL) {
        MozNPN_RetainObject(m_appletNPObject);
    }

    return m_appletNPObject;
}

// This method is called to update the applet's location -- used only on Mac OS X
void MozPluginInstance::updateLocationAndClip() {
    assert(m_pluginObject != NULL);
    if (m_pluginObject == NULL) {
        // Something went wrong during initialization
        return;
    }

    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    // Guard against crashes in product builds
    if (env == NULL || g_initFailed) { 
        return;
    }
    env->CallVoidMethod(m_pluginObject, mozPluginUpdateLocationAndClipID);
    CLEAR_EXCEPTION(env);
}
