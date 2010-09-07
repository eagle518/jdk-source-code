/*
 * @(#)JavaJSObject.cpp	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "JavaJSObject.h"
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSValueRef.h>
#include <string.h>

#include "AbstractPlugin.h"
#include "JNIExceptions.h"
#include "LocalFramePusher.h"

// Uncomment for debugging output
// #define DEBUG

// This is a temporary hack for the WWDC demo and the associated
// Safari patch and must be removed (FIXME)
#define WWDC_HACK

static JSClassRef javaObjectClass = NULL;
static jmethodID stringToIdentifierID = NULL;

struct PerObjectData {
    // This is a JNI global ref set up at object creation time
    jobject parentJavaPlugin;
    // This is the target Java object of this one
    jobject targetObject;
    // If this JavaObject actually represents a Java namespace (i.e., the "Packages" object),
    // this will be set initially, and the target object will be null
    char* javaNameSpace;
    // If this JavaObject actually represents a Java method name (see
    // GetProperty, below), this will be set, and everything above
    // will be null
    jstring methodName;
};

static void resolveJavaNameSpace(PerObjectData* data) {
#ifdef DEBUG
    printf("JavaJSObject.cpp: resolveJavaNameSpace for %p\n", (void*) data);
#endif
    if (data->targetObject == NULL && data->javaNameSpace != NULL) {
        data->targetObject =
            AbstractPlugin::getJavaNameSpace(data->parentJavaPlugin,
                                             data->javaNameSpace);
    }
}

static char* jsStringToCString(JSStringRef jsStr) {
    size_t sz = JSStringGetMaximumUTF8CStringSize(jsStr);
    char* buf = (char*) malloc(sz);
    JSStringGetUTF8CString(jsStr, buf, sz);
    return buf;
}

static jstring jsStringToJString(JNIEnv* env, JSStringRef jsStr) {
    char* str = jsStringToCString(jsStr);
    jstring jstr = env->NewStringUTF(str);
    free(str);
    return jstr;
}

static jlong jsStringToIdentifier(JNIEnv* env, JSStringRef jsStr, jobject parentSafariPlugin) {
    jstring jstr = jsStringToJString(env, jsStr);
    jlong identifier = env->CallLongMethod(parentSafariPlugin,
                                           stringToIdentifierID,
                                           jstr);
    CLEAR_EXCEPTION(env);
    return identifier;
}


static void JavaJSObject_Initialize(JSContextRef ctx, JSObjectRef object) {
    // Nothing to do -- setting up of plugin already handled in caller of JSObjectMake
}

static void JavaJSObject_Finalize(JSObjectRef object) {
    PerObjectData* data = (PerObjectData*) JSObjectGetPrivate(object);
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    if (data->parentJavaPlugin != NULL) {
        env->DeleteGlobalRef(data->parentJavaPlugin);
        data->parentJavaPlugin = NULL;
    }
    if (data->targetObject != NULL) {
        env->DeleteGlobalRef(data->targetObject);
        data->targetObject = NULL;
    }
    if (data->javaNameSpace != NULL) {
        free(data->javaNameSpace);
        data->javaNameSpace = NULL;
    }
    if (data->methodName != NULL) {
        env->DeleteGlobalRef(data->methodName);
        data->methodName = NULL;
    }
    free(data);
}

void JavaJSObject_ThrowException(JSContextRef ctx, JSStringRef exceptionString, JSValueRef* exception) {
#ifdef DEBUG
    printf("JavaJSObject_ThrowException\n");
#endif

    if (exception == NULL) {
        return;
    }

    // Fabricate an Error object and mutate its message for convenience
    // (Don't want to have to worry about quoting rules)
    JSValueRef excObj = JSEvaluateScript(ctx,
                                      JSStringCreateWithUTF8CString("new Error(\"\");"),
                                      NULL,
                                      NULL,
                                      0,
                                      NULL);
    if (excObj == NULL) {
#ifdef DEBUG
        printf("   error allocating exception object for throwing\n");
#endif
        return;
    }

    // Set the message field of the exception object
    JSObjectSetProperty(ctx,
                        (JSObjectRef) excObj,
                        JSStringCreateWithUTF8CString("message"),
                        JSValueMakeString(ctx, exceptionString),
                        kJSPropertyAttributeNone,
                        NULL);
    
#ifdef DEBUG
    printf("   set message property on exception\n");
#endif

    *exception = excObj;

#ifdef DEBUG
    printf("   set up exception for throwing\n");
#endif
}

static void throwException(JSContextRef ctx, JSValueRef* exception, const char* exceptionString) {
#ifdef DEBUG
    printf("JavaJSObject.cpp: ");
    printf(exceptionString);
    printf("\n");
#endif

    if (exception == NULL) {
        return;
    }

    JSStringRef excStr = JSStringCreateWithUTF8CString(exceptionString);
    if (excStr == NULL) {
#ifdef DEBUG
        printf("JavaJSObject.cpp: error allocating exception string\n");
#endif
        return;
    }

    JavaJSObject_ThrowException(ctx, excStr, exception);
}

static bool sanityCheck(JSContextRef ctx, PerObjectData* data, JSValueRef* exception) {
    if (data == NULL) {
        throwException(ctx, exception, "NULL private data on JavaJSObject");
        return false;
    }
    if (data->parentJavaPlugin == NULL) {
        throwException(ctx, exception, "NULL parent Java Plug-In");
        return false;
    }
    if (stringToIdentifierID == NULL) {
        throwException(ctx, exception, "NULL stringToIdentifierID method on SafariPlugin");
        return false;
    }
    return true;
}

// The following two functions are private to the implementation
static JSValueRef JavaJSObject_AllocateForMethodName(JSContextRef ctx, JNIEnv* env, jstring methodName)
{
    PerObjectData* data = (PerObjectData*) calloc(1, sizeof(PerObjectData));
    data->methodName = (jstring) env->NewGlobalRef(methodName);
    JSObjectRef obj = JSObjectMake(ctx, javaObjectClass, data);
    // A JSObjectRef is a JSValueRef -- see JSBase.h
    // Note that reference counting of this value is handled at a higher level
    return (JSValueRef) obj;
}

static jstring JavaJSObject_GetMethodName(JSObjectRef object) {
    if (object == NULL) {
        return NULL;
    }
    PerObjectData* data = (PerObjectData*) JSObjectGetPrivate(object);
    if (data == NULL) {
        return NULL;
    }
    return data->methodName;
}

static bool JavaJSObject_HasProperty(JSContextRef ctx,
                                     JSObjectRef object,
                                     JSStringRef propertyName) {
#ifdef DEBUG
    printf("JavaJSObject_HasProperty entered: object = %p\n", (void*) object);
#endif
    PerObjectData* data = (PerObjectData*) JSObjectGetPrivate(object);
    if (!sanityCheck(ctx, data, NULL)) {
#ifdef DEBUG
        printf("   sanity check failed\n");
#endif
        return NULL;
    }
    resolveJavaNameSpace(data);
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    jlong identifier = jsStringToIdentifier(env, propertyName, data->parentJavaPlugin);
    CHECK_EXCEPTION_VAL(env, false);

    // Due to the semantics of the Safari JavaScript interpreter (see
    // below), we have to return true from this routine if the object
    // has a method of this name as well. See below.

    if (AbstractPlugin::javaObjectHasMethod(data->parentJavaPlugin,
                                            data->targetObject,
                                            identifier)) {
        return true;
    }

    return AbstractPlugin::javaObjectHasField(data->parentJavaPlugin,
                                              data->targetObject,
                                              identifier);
}

static JSValueRef JavaJSObject_GetProperty(JSContextRef ctx,
                                           JSObjectRef object,
                                           JSStringRef propertyName,
                                           JSValueRef* exception) {
#ifdef DEBUG
    printf("JavaJSObject_GetProperty entered: object = %p\n", (void*) object);
#endif
    PerObjectData* data = (PerObjectData*) JSObjectGetPrivate(object);
    if (!sanityCheck(ctx, data, exception)) {
#ifdef DEBUG
        printf("   sanity check failed\n");
#endif
        return NULL;
    }
    resolveJavaNameSpace(data);
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    jlong identifier = jsStringToIdentifier(env, propertyName, data->parentJavaPlugin);
    CHECK_EXCEPTION_VAL(env, NULL);
    JSValueRef result;
    // The JavaScriptCore API is different than the other JavaScript
    // engines to which the new Java Plug-In has been ported. Method
    // invocation occurs in two steps: the fetching of the property
    // corresponding to the method, and invocation of that method
    // (calling it as a function). In order to implement this without
    // completely changing our LiveConnect message passing
    // infrastructure, we use our existing HAS_METHOD JavaObjectOp
    // query to see whether this property fetch appears to be for a
    // method, and if so, we return an object which we can later use
    // during the method invocation to figure out the associated
    // method name. (We would otherwise need to reify methods across
    // the client/server process boundary.) This has the implication
    // that if we have a field and method with the same name, the
    // method shadows the field in the Safari JavaScript engine. This
    // is inevitable because we don't know whether the GetProperty
    // request coming in is intended for simply a property fetch or
    // whether it is on behalf of an upcoming method invocation.
#ifdef DEBUG
    printf("   calling AbstractPlugin::javaObjectHasMethod\n");
#endif
    if (AbstractPlugin::javaObjectHasMethod(data->parentJavaPlugin,
                                            data->targetObject,
                                            identifier)) {
        JSValueRef res = JavaJSObject_AllocateForMethodName(ctx, env, jsStringToJString(env, propertyName));
#ifdef DEBUG
        printf("   detected GetProperty for method -- returning JSObject %p for method name\n", (void*) res);
#endif
        return res;
    }

#ifdef DEBUG
    printf("   calling AbstractPlugin::javaObjectGetField\n");
#endif
    if (!AbstractPlugin::javaObjectGetField(data->parentJavaPlugin,
                                            data->targetObject,
                                            JNI_FALSE, // Don't care in this implementation
                                            identifier,
                                            (jlong) &result,
                                            (jlong) exception)) {
#ifdef DEBUG
        printf("   returning NULL\n");
#endif
        return NULL;
    }
#ifdef DEBUG
    printf("   returning result\n");
#endif
    return result;
}

static bool JavaJSObject_SetProperty(JSContextRef ctx,
                                     JSObjectRef object,
                                     JSStringRef propertyName,
                                     JSValueRef value,
                                     JSValueRef* exception) {
    PerObjectData* data = (PerObjectData*) JSObjectGetPrivate(object);
    if (!sanityCheck(ctx, data, exception)) {
        return false;
    }
    resolveJavaNameSpace(data);
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    jlong identifier = jsStringToIdentifier(env, propertyName, data->parentJavaPlugin);
    CHECK_EXCEPTION_VAL(env, false);
    if (!AbstractPlugin::javaObjectSetField(data->parentJavaPlugin,
                                            data->targetObject,
                                            JNI_FALSE, // Don't care in this implementation
                                            identifier,
                                            (jlong) &value,
                                            (jlong) exception)) {
        return false;
    }
    return true;
}

static bool JavaJSObject_DeleteProperty(JSContextRef ctx,
                                        JSObjectRef object,
                                        JSStringRef propertyName,
                                        JSValueRef* exception) {
    PerObjectData* data = (PerObjectData*) JSObjectGetPrivate(object);
    if (!sanityCheck(ctx, data, exception)) {
        return false;
    }
    // Only passing this up to Java to keep all localization in Java
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    jlong identifier = jsStringToIdentifier(env, propertyName, data->parentJavaPlugin);
    CHECK_EXCEPTION_VAL(env, false);
    AbstractPlugin::javaObjectRemoveField(data->parentJavaPlugin,
                                          data->targetObject,
                                          identifier,
                                          (jlong) exception);
    return false;
}

static JSValueRef JavaJSObject_CallAsFunction(JSContextRef ctx,
                                              JSObjectRef function,
                                              JSObjectRef object,
                                              size_t argumentCount,
                                              const JSValueRef* arguments,
                                              JSValueRef* exception) {
#ifdef DEBUG
    printf("JavaJSObject_CallAsFunction entered: object = %p, function = %p\n", (void*) object, (void*) function);
#endif
    PerObjectData* data = (PerObjectData*) JSObjectGetPrivate(object);
    if (!sanityCheck(ctx, data, exception)) {
#ifdef DEBUG
        printf("   first sanity check failed\n");
#endif
#ifdef WWDC_HACK
        // Try again
        *exception = NULL;
        object = (JSObjectRef) (((char*) object) + 0x20);
#ifdef DEBUG
        printf("   trying to fetch private data for object at %p\n", (void*) object);
#endif
        data = (PerObjectData*) JSObjectGetPrivate(object);
        if (!sanityCheck(ctx, data, exception)) {
#ifdef DEBUG
            printf("   second sanity check failed\n");
#endif
            // Try again
            *exception = NULL;
            object = (JSObjectRef) (((char*) object) - 0x40);
#ifdef DEBUG
            printf("   trying to fetch private data for object at %p\n", (void*) object);
#endif
            data = (PerObjectData*) JSObjectGetPrivate(object);
            if (!sanityCheck(ctx, data, exception)) {
#ifdef DEBUG
                printf("   third and last sanity check failed\n");
#endif
                return NULL;
            }
        }
#else  // WWDC_HACK
        return NULL;
#endif // WWDC_HACK
    }
    if (!JSValueIsObjectOfClass(ctx, function, javaObjectClass)) {
#ifdef DEBUG
        printf("   incoming function is of wrong type\n");
#endif
        throwException(ctx, exception, "Non-Java function passed to JavaJSObject_CallAsFunction");
        return NULL;
    }
    resolveJavaNameSpace(data);
#ifdef DEBUG
    printf("   fetching method name\n");
#endif
    jstring methodName = JavaJSObject_GetMethodName(function);
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
#ifdef DEBUG
    printf("   looking up method identifier\n");
#endif
    jlong identifier = env->CallLongMethod(data->parentJavaPlugin,
                                           stringToIdentifierID,
                                           methodName);
    if (env->ExceptionOccurred()) {
        CLEAR_EXCEPTION(env);
        throwException(ctx, exception, "Exception occurred during method identifier lookup");
        return NULL;
    }
    JSValueRef result = NULL;
#ifdef DEBUG
    printf("   doing method invocation\n");
#endif
    bool res = AbstractPlugin::javaObjectInvoke(data->parentJavaPlugin,
                                                data->targetObject,
                                                JNI_FALSE, // Don't care in this implementation
                                                identifier,
                                                (jlong) arguments,
                                                argumentCount,
                                                (jlong) &result,
                                                (jlong) exception);
    if (res) {
        return result;
    }
    return NULL;
}

static JSObjectRef JavaJSObject_CallAsConstructor(JSContextRef ctx,
                                                  JSObjectRef constructor,
                                                  size_t argumentCount,
                                                  const JSValueRef* arguments,
                                                  JSValueRef* exception) {
#ifdef DEBUG
    printf("JavaJSObject_CallAsConstructor entered: constructor = %p\n", (void*) constructor);
#endif
    PerObjectData* data = (PerObjectData*) JSObjectGetPrivate(constructor);
    if (!sanityCheck(ctx, data, exception)) {
#ifdef DEBUG
        printf("   sanity check failed\n");
#endif
        return NULL;
    }
    resolveJavaNameSpace(data);
    LocalFramePusher pusher;
    JSValueRef result = NULL;
#ifdef DEBUG
    printf("   doing constructor invocation\n");
#endif
    bool res = AbstractPlugin::javaObjectInvokeConstructor(data->parentJavaPlugin,
                                                           data->targetObject,
                                                           JNI_FALSE, // Don't care in this implementation
                                                           (jlong) arguments,
                                                           argumentCount,
                                                           (jlong) &result,
                                                           (jlong) exception);
    if (res) {
        return JSValueToObject(ctx, result, exception);
    }
    return NULL;
}

static JSValueRef JavaJSObject_ConvertToType(JSContextRef ctx,
                                             JSObjectRef object,
                                             JSType type,
                                             JSValueRef* exception) {
    if (type != kJSTypeString) {
        // We only support the toString operation here
    }
    PerObjectData* data = (PerObjectData*) JSObjectGetPrivate(object);
    if (!sanityCheck(ctx, data, exception)) {
        return NULL;
    }
    resolveJavaNameSpace(data);
    JSValueRef result = NULL;
    bool res = AbstractPlugin::javaObjectInvoke(data->parentJavaPlugin,
                                                data->targetObject,
                                                JNI_FALSE, // Don't care in this implementation
                                                0,         // Special-cased at the Java level
                                                0,         // No arguments
                                                0,
                                                (jlong) &result,
                                                (jlong) exception);
    if (!res) {
        return NULL;
    }
    return result;
}

//----------------------------------------------------------------------
// Public entry points
//

void JavaJSObject_GlobalInitialize() {
    // Look up methods we need
    LocalFramePusher pusher;
    JNIEnv* env = pusher.getEnv();
    jclass safariPluginClass = env->FindClass("sun/plugin2/main/server/SafariPlugin");
    if (safariPluginClass == NULL) {
        env->ExceptionDescribe();
    }
    assert(safariPluginClass != NULL);
    if (safariPluginClass == NULL) {
        return;
    }
    stringToIdentifierID = env->GetMethodID(safariPluginClass, "stringToIdentifier", "(Ljava/lang/String;)J");
    CLEAR_EXCEPTION(env);
    assert(stringToIdentifierID != NULL);
    
    JSClassDefinition classDef;
    classDef.version = 0;
    classDef.attributes = kJSClassAttributeNone;
    classDef.className = "JavaJSObject";
    classDef.parentClass = NULL;
    classDef.staticValues = NULL;
    classDef.staticFunctions = NULL;
    classDef.initialize = &JavaJSObject_Initialize;
    classDef.finalize = &JavaJSObject_Finalize;
    classDef.hasProperty = &JavaJSObject_HasProperty;
    classDef.getProperty = &JavaJSObject_GetProperty;
    classDef.setProperty = &JavaJSObject_SetProperty;
    classDef.deleteProperty = &JavaJSObject_DeleteProperty;
    // We don't support enumeration of properties at this time
    classDef.getPropertyNames = NULL;
    classDef.callAsFunction = &JavaJSObject_CallAsFunction;
    classDef.callAsConstructor = &JavaJSObject_CallAsConstructor;
    // FIXME: we do not (yet) support instanceof checks against
    // objects created via the CallAsConstructor callback
    classDef.hasInstance = NULL;
    classDef.convertToType = &JavaJSObject_ConvertToType;
    javaObjectClass = JSClassCreate(&classDef);
    assert(javaObjectClass != NULL);
    if (javaObjectClass == NULL) {
        printf("JavaJSObject_GlobalInitialize: JSClassCreate failed\n");
    }
}

JSValueRef JavaJSObject_Allocate(JSContextRef ctx, JNIEnv* env, jobject parentJavaPlugin, jobject targetObject)
{
    PerObjectData* data = (PerObjectData*) calloc(1, sizeof(PerObjectData));
    data->parentJavaPlugin = env->NewGlobalRef(parentJavaPlugin);
    data->targetObject = env->NewGlobalRef(targetObject);
    JSObjectRef obj = JSObjectMake(ctx, javaObjectClass, data);
    // A JSObjectRef is a JSValueRef -- see JSBase.h
    // Note that reference counting of this value is handled at a higher level
    return (JSValueRef) obj;
}

JSValueRef JavaJSObject_AllocateForNameSpace(JSContextRef ctx, JNIEnv* env, jobject parentJavaPlugin, const char* nameSpace)
{
    PerObjectData* data = (PerObjectData*) calloc(1, sizeof(PerObjectData));
    data->parentJavaPlugin = env->NewGlobalRef(parentJavaPlugin);
    data->javaNameSpace = strdup(nameSpace);
    JSObjectRef obj = JSObjectMake(ctx, javaObjectClass, data);
    // A JSObjectRef is a JSValueRef -- see JSBase.h
    // Note that reference counting of this value is handled at a higher level
    return (JSValueRef) obj;
}

JSClassRef JavaJSObject_GetClass()
{
    return javaObjectClass;
}

jobject JavaJSObject_GetTargetObject(JSObjectRef object) {
    if (object == NULL) {
        return NULL;
    }
    PerObjectData* data = (PerObjectData*) JSObjectGetPrivate(object);
    if (data == NULL) {
        return NULL;
    }
    return data->targetObject;
}
