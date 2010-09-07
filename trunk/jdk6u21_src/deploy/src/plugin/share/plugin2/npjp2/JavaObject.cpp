/*
 * @(#)JavaObject.cpp	1.8 10/03/24 12:03:40
 *
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "StdAfx.h"

#include "JavaObject.h"
#include "MozPluginInstance.h"
#include "MozExports.h"
#include "AbstractPlugin.h"
#include "JavaVM.h"

// The NPClass associated with our Java objects
static NPClass javaObjectClass;

extern "C" {

// Helper function to resolve Java name spaces for JavaObjects
static void resolveJavaNameSpace(JavaObject* obj) {
    if (obj->targetObject == NULL && obj->javaNameSpace != NULL) {
        obj->targetObject =
            AbstractPlugin::getJavaNameSpace(obj->parentJavaPlugin,
                                             obj->javaNameSpace);
    }
}

// For debugging
static void printIdentifier(NPIdentifier identifier) {
    if (MozNPN_IdentifierIsString(identifier)) {
        const NPUTF8* chars = MozNPN_UTF8FromIdentifier(identifier);
        printf("String identifier: %s\n", chars);
    } else {
        printf("Int identifier: %d\n", MozNPN_IntFromIdentifier(identifier));
    }
}

NPObject *JavaObject_Allocate(NPP npp, NPClass *aClass) {
    JavaObject* res    = new JavaObject();
    JNIEnv* env = JavaVM_GetJNIEnv();
    MozPluginInstance* plugin = (MozPluginInstance*) npp->pdata;
    res->parentJavaPlugin  = env->NewGlobalRef(plugin->getJavaMozillaPluginInstance());
    res->targetObject  = NULL;
    res->javaNameSpace = NULL;
    return res;
}

void JavaObject_Deallocate(NPObject *npobj) {
    JavaObject* obj = (JavaObject*) npobj;
    if (obj->targetObject != NULL && obj->parentJavaPlugin != NULL) {
        // Should probably assert that the parentJavaPlugin is non-NULL in debug builds
        AbstractPlugin::releaseRemoteJavaObject(obj->parentJavaPlugin, obj->targetObject);
        obj->targetObject = NULL;
    }
    if (obj->parentJavaPlugin != NULL) {
        JNIEnv* env = JavaVM_GetJNIEnv();
        env->DeleteGlobalRef(obj->parentJavaPlugin);
        obj->parentJavaPlugin = NULL;
    }
    if (obj->javaNameSpace != NULL) {
        ::free(obj->javaNameSpace);
        obj->javaNameSpace = NULL;
    }
    delete obj;
}

void JavaObject_Invalidate(NPObject *npobj) {
    // Don't think we need to pay attention to this notification as
    // long as we get a chance to clean up in Deallocate
}

bool JavaObject_HasMethod(NPObject *npobj, NPIdentifier name) {
    JavaObject* obj = (JavaObject*) npobj;
    resolveJavaNameSpace(obj);
    return AbstractPlugin::javaObjectHasMethod(obj->parentJavaPlugin,
                                               obj->targetObject,
                                               (jlong) name);
}

bool JavaObject_Invoke(NPObject *npobj, NPIdentifier name,
                       const NPVariant *args, uint32_t argCount,
                       NPVariant *result) {
    JavaObject* obj = (JavaObject*) npobj;
    resolveJavaNameSpace(obj);
    return AbstractPlugin::javaObjectInvoke(obj->parentJavaPlugin,
                                            obj->targetObject,
                                            JNI_FALSE, // Don't care in this implementation
                                            (jlong) name,
                                            (jlong) args,
                                            argCount,
                                            (jlong) result,
                                            (jlong) npobj);
}

bool JavaObject_Construct(NPObject *npobj,
                              const NPVariant *args,
                              uint32_t argCount,
                              NPVariant *result) {
    JavaObject* obj = (JavaObject*) npobj;
    resolveJavaNameSpace(obj);
    return AbstractPlugin::javaObjectInvokeConstructor(obj->parentJavaPlugin,
                                                       obj->targetObject,
                                                       JNI_FALSE, // Don't care in this implementation
                                                       (jlong) args,
                                                       argCount,
                                                       (jlong) result,
                                                       (jlong) npobj);
}

bool JavaObject_HasProperty(NPObject *npobj, NPIdentifier name) {
    JavaObject* obj = (JavaObject*) npobj;
    resolveJavaNameSpace(obj);
    return AbstractPlugin::javaObjectHasField(obj->parentJavaPlugin,
                                              obj->targetObject,
                                              (jlong) name);
}

bool JavaObject_GetProperty(NPObject *npobj, NPIdentifier name,
                            NPVariant *result) {
    JavaObject* obj = (JavaObject*) npobj;
    resolveJavaNameSpace(obj);
    return AbstractPlugin::javaObjectGetField(obj->parentJavaPlugin,
                                              obj->targetObject,
                                              JNI_FALSE, // Don't care in this implementation
                                              (jlong) name,
                                              (jlong) result,
                                              (jlong) npobj);
}

bool JavaObject_SetProperty(NPObject *npobj, NPIdentifier name,
                            const NPVariant *value) {
    JavaObject* obj = (JavaObject*) npobj;
    return AbstractPlugin::javaObjectSetField(obj->parentJavaPlugin,
                                              obj->targetObject,
                                              JNI_FALSE, // Don't care in this implementation
                                              (jlong) name,
                                              (jlong) value,
                                              (jlong) npobj);
}

bool JavaObject_RemoveProperty(NPObject *npobj,
                               NPIdentifier name) {
    JavaObject* obj = (JavaObject*) npobj;
    // Only handing this over in order to keep all localization up in Java
    AbstractPlugin::javaObjectRemoveField(obj->parentJavaPlugin,
                                          obj->targetObject,
                                          (jlong) name,
                                          (jlong) npobj);
    return false;
}

} // extern "C"

// One-time initialization
void JavaObject::initialize() {
    javaObjectClass.structVersion  = NP_CLASS_STRUCT_VERSION;
    javaObjectClass.allocate       = &JavaObject_Allocate;
    javaObjectClass.deallocate     = &JavaObject_Deallocate;
    javaObjectClass.hasMethod      = &JavaObject_HasMethod;
    javaObjectClass.invoke         = &JavaObject_Invoke;
    javaObjectClass.invokeDefault  = NULL;
    javaObjectClass.hasProperty    = &JavaObject_HasProperty;
    javaObjectClass.getProperty    = &JavaObject_GetProperty;
    javaObjectClass.setProperty    = &JavaObject_SetProperty;
    javaObjectClass.removeProperty = &JavaObject_RemoveProperty;
    javaObjectClass.enumerate      = NULL;
    javaObjectClass.construct      = &JavaObject_Construct;
}

// Allocation of new JavaObjects
JavaObject* JavaObject::allocate(NPP npp, jobject obj) {
    // NOTE: We need to return an object with a zero reference count
    // from this method. The caller, which is likely indirectly be
    // the web browser, is responsible for incrementing the reference
    // count of the returned object the first time.
    JavaObject* res = (JavaObject*) MozNPN_CreateObject(npp, &javaObjectClass);
    res->referenceCount = 0;
    res->targetObject = obj;
    return res;
}

JavaObject* JavaObject::allocateForJavaNameSpace(NPP npp, const char* javaNameSpace) {
    JavaObject* res = (JavaObject*) MozNPN_CreateObject(npp, &javaObjectClass);
    res->javaNameSpace = strdup(javaNameSpace);
    return res;
}

bool JavaObject::isJavaObject(NPObject* obj) {
    // This is an easy type test
    return (obj->_class == &javaObjectClass);
}
