/*
 * @(#)SafariPlugin.m	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#import <WebKit/WebKit.h>
#import <WebKit/WebPlugin.h>
#import <WebKit/WebPluginContainer.h>
#import <assert.h>

#import <JavaScriptCore/JSBase.h>
#import <JavaScriptCore/JSStringRef.h>
#import <stdbool.h>

#import "JavaPlugin2Safari.h"
#import "JavaJSObject.h"
#import "StringUtil.h"
#import "AbstractPlugin.h"
#import "JNIExceptions.h"
#import "sun_plugin2_main_server_SafariPlugin.h"

// Native methods for sun.plugin2.main.server.SafariPlugin

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_webPlugInInitialize0
  (JNIEnv *env, jclass unused)
{
    JavaJSObject_GlobalInitialize();
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_updateLocationAndClip
  (JNIEnv *env, jobject unused, jlong safariPlugin)
{
    JavaPlugIn2Safari* plugin = (JavaPlugIn2Safari*) safariPlugin;
    [plugin updateLocationAndClip];
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_nsObjectRetain
  (JNIEnv *env, jclass unused, jlong jobj)
{
    NSObject* obj = (NSObject*) jobj;
    [obj retain];
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_nsObjectRelease
  (JNIEnv *env, jclass unused, jlong jobj)
{
    NSObject* obj = (NSObject*) jobj;
    [obj release];
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_jsValueProtect
  (JNIEnv *env, jclass unused, jlong webPlugInContainer, jlong jval)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    if (container == nil)
        return;
    WebFrame* frame = [container webFrame];
    JSValueProtect([frame globalContext], (JSValueRef) jval);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_jsValueUnprotect
  (JNIEnv *env, jclass unused, jlong webPlugInContainer, jlong jval)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    if (container == nil)
        return;
    WebFrame* frame = [container webFrame];
    JSValueUnprotect([frame globalContext], (JSValueRef) jval);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_invokeLater0
  (JNIEnv *env, jobject unused, jlong safariPlugin)
{
    JavaPlugIn2Safari* plugin = (JavaPlugIn2Safari*) safariPlugin;
    [plugin performSelectorOnMainThread: @selector(drainRunnableQueue) withObject: nil waitUntilDone: NO];
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_showDocument0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jstring jurl, jstring jtarget)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    NSString* urlString = jstringToNSString(env, jurl);
    NSURL* url = [NSURL URLWithString: urlString];
    NSURLRequest* req = [NSURLRequest requestWithURL: url];
    NSString* targetString = nil;
    if (jtarget != NULL) {
        targetString = jstringToNSString(env, jtarget);
    }
    [container webPlugInContainerLoadRequest: req inFrame: targetString];
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_showStatus0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jstring jstatus)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    NSString* status = jstringToNSString(env, jstatus);
    [container webPlugInContainerShowStatus: status];
}

static jstring jsStringToJString(JNIEnv* env, JSStringRef jsStr) {
    const JSChar* chars = JSStringGetCharactersPtr(jsStr);
    return env->NewString(chars, JSStringGetLength(jsStr));
}

static JSStringRef jstringToJSString(JNIEnv* env, jstring jstr) {
    const jchar* chars = env->GetStringChars(jstr, NULL);
    JSStringRef jsstr = JSStringCreateWithCharacters(chars, env->GetStringLength(jstr));
    env->ReleaseStringChars(jstr, chars);
    return jsstr;
}

static void exceptionToString(JNIEnv* env, JSContextRef ctx, JSValueRef exception, jobjectArray exceptionString) {
    if (exception != NULL) {
        JSStringRef excStr = JSValueToStringCopy(ctx, exception, NULL);
        if (excStr != NULL) {
            env->SetObjectArrayElement(exceptionString, 0, jsStringToJString(env, excStr));
        }
    }
}

JNIEXPORT jboolean JNICALL Java_sun_plugin2_main_server_SafariPlugin_javaScriptCall0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong valueRef, jstring jname,
   jlong jargArray, jint jargArrayLen, jlong jresArray, jobjectArray exceptionString)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSValueRef obj = (JSValueRef) valueRef;
    JSStringRef name = jstringToJSString(env, jname);
    JSValueRef exception = NULL;

    // FIXME: refactor this in terms of javaScriptGetMember

    // Fetch the actual receiver
    JSObjectRef receiver = JSValueToObject(ctx, obj, &exception);
    if (receiver == NULL && exception != NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }

    // Look up this method in the receiver's properties
    JSValueRef methodRef = JSObjectGetProperty(ctx, receiver, name, &exception);
    if (JSValueIsUndefined(ctx, methodRef)) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }
    JSObjectRef method = JSValueToObject(ctx, methodRef, &exception);
    if (method == NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }

    // Call the method
    JSValueRef result = JSObjectCallAsFunction(ctx,
                                               method,
                                               receiver,
                                               jargArrayLen,
                                               (JSValueRef*) jargArray,
                                               &exception);
    if (result == NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }
    *((JSValueRef*) jresArray) = result;
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_sun_plugin2_main_server_SafariPlugin_javaScriptEval0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong valueRef, jstring jcode, jlong jresArray, jobjectArray exceptionString)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSValueRef obj = (JSValueRef) valueRef;
    JSStringRef code = jstringToJSString(env, jcode);
    JSValueRef exception = NULL;

    // Fetch the actual receiver
    JSObjectRef receiver = JSValueToObject(ctx, obj, &exception);
    if (receiver == NULL && exception != NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }

    // Perform the evaluation
    JSValueRef result = JSEvaluateScript(ctx, code, receiver,
                                         NULL, 0, &exception);
    if (result == NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }
    *((JSValueRef*) jresArray) = result;
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_sun_plugin2_main_server_SafariPlugin_javaScriptGetMember0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong valueRef, jstring jname, jlong jresArray, jobjectArray exceptionString)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSValueRef obj = (JSValueRef) valueRef;
    JSStringRef name = jstringToJSString(env, jname);
    JSValueRef exception = NULL;

    // Fetch the actual receiver
    JSObjectRef receiver = JSValueToObject(ctx, obj, &exception);
    if (receiver == NULL && exception != NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }

    // Fetch the property
    JSValueRef result = JSObjectGetProperty(ctx, receiver, name, &exception);
    if (JSValueIsUndefined(ctx, result)) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }
    *((JSValueRef*) jresArray) = result;
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_sun_plugin2_main_server_SafariPlugin_javaScriptSetMember0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong valueRef, jstring jname, jlong jargArray, jobjectArray exceptionString)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSValueRef obj = (JSValueRef) valueRef;
    JSStringRef name = jstringToJSString(env, jname);
    JSValueRef* args = (JSValueRef*) jargArray;
    JSValueRef exception = NULL;

    // Fetch the actual receiver
    JSObjectRef receiver = JSValueToObject(ctx, obj, &exception);
    if (receiver == NULL && exception != NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }

    // Set the property
    JSObjectSetProperty(ctx, receiver, name, args[0],
                        kJSPropertyAttributeNone, &exception);
    if (exception != NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_sun_plugin2_main_server_SafariPlugin_javaScriptRemoveMember0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong valueRef, jstring jname, jobjectArray exceptionString)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSValueRef obj = (JSValueRef) valueRef;
    JSStringRef name = jstringToJSString(env, jname);
    JSValueRef exception = NULL;

    // Fetch the actual receiver
    JSObjectRef receiver = JSValueToObject(ctx, obj, &exception);
    if (receiver == NULL && exception != NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }

    // Remove the member
    bool result = JSObjectDeleteProperty(ctx, receiver, name, &exception);
    if (!result) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_sun_plugin2_main_server_SafariPlugin_javaScriptGetSlot0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong valueRef, jint index, jlong jresArray, jobjectArray exceptionString)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSValueRef obj = (JSValueRef) valueRef;
    JSValueRef exception = NULL;

    // Fetch the actual receiver
    JSObjectRef receiver = JSValueToObject(ctx, obj, &exception);
    if (receiver == NULL && exception != NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }

    // Fetch the property
    JSValueRef result = JSObjectGetPropertyAtIndex(ctx, receiver, index, &exception);
    if (JSValueIsUndefined(ctx, result)) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }
    *((JSValueRef*) jresArray) = result;
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_sun_plugin2_main_server_SafariPlugin_javaScriptSetSlot0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong valueRef, jint index, jlong jargArray, jobjectArray exceptionString)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSValueRef obj = (JSValueRef) valueRef;
    JSValueRef* args = (JSValueRef*) jargArray;
    JSValueRef exception = NULL;

    // Fetch the actual receiver
    JSObjectRef receiver = JSValueToObject(ctx, obj, &exception);
    if (receiver == NULL && exception != NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }

    // Set the property
    JSObjectSetPropertyAtIndex(ctx, receiver, index, args[0], &exception);
    if (exception != NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jstring JNICALL Java_sun_plugin2_main_server_SafariPlugin_javaScriptToString0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong valueRef, jobjectArray exceptionString)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSValueRef exception = NULL;
    
    JSStringRef result = JSValueToStringCopy(ctx, (JSValueRef) valueRef, &exception);
    if (result == NULL) {
        exceptionToString(env, ctx, exception, exceptionString);
        return NULL;
    }
    return jsStringToJString(env, result);
}

JNIEXPORT jlong JNICALL Java_sun_plugin2_main_server_SafariPlugin_allocateVariantArray0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jint length)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSValueRef* res = (JSValueRef*) malloc(length * sizeof(JSValueRef));
    // Fill it with undefined values
    for (int i = 0; i < length; i++) {
        res[i] = JSValueMakeUndefined(ctx);
    }
    return (jlong) res;
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_freeVariantArray
  (JNIEnv *env, jobject unused, jlong jarr, jint size)
{
    // FIXME: should we be increasing the reference count of each
    // element we store in this array, and releasing each element
    // before freeing?
    free((JSValueRef*) jarr);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElement0__JJIZ
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index, jboolean val)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    ((JSValueRef*) jarr)[index] = JSValueMakeBoolean(ctx, val);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElement0__JJIB
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index, jbyte val)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    ((JSValueRef*) jarr)[index] = JSValueMakeNumber(ctx, val);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElement0__JJIC
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index, jchar val)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    ((JSValueRef*) jarr)[index] = JSValueMakeNumber(ctx, val);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElement0__JJIS
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index, jshort val)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    ((JSValueRef*) jarr)[index] = JSValueMakeNumber(ctx, val);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElement0__JJII
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index, jint val)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    ((JSValueRef*) jarr)[index] = JSValueMakeNumber(ctx, val);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElement0__JJIJ
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index, jlong val)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    ((JSValueRef*) jarr)[index] = JSValueMakeNumber(ctx, val);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElement0__JJIF
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index, jfloat val)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    ((JSValueRef*) jarr)[index] = JSValueMakeNumber(ctx, val);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElement0__JJID
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index, jdouble val)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    ((JSValueRef*) jarr)[index] = JSValueMakeNumber(ctx, val);
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElement0__JJILjava_lang_String_2
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index, jstring val)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    ((JSValueRef*) jarr)[index] = JSValueMakeString(ctx, jstringToJSString(env, val));
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElementToScriptingObject0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index, jlong val)
{
    // FIXME: do we need to consider incrementing the reference count here?
    ((JSValueRef*) jarr)[index] = (JSValueRef) val;    
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_setVariantArrayElementToVoid0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong jarr, jint index)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    // Unclear whether we should use undefined or null for this
    ((JSValueRef*) jarr)[index] = JSValueMakeNull(ctx);
}

JNIEXPORT jobject JNICALL Java_sun_plugin2_main_server_SafariPlugin_variantArrayElementToObject0
  (JNIEnv *env, jobject plugInObj, jlong webPlugInContainer, jlong jarr, jint index)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSValueRef* arr = (JSValueRef*) jarr;
    JSValueRef val = arr[index];

    if (val == NULL ||
        JSValueIsUndefined(ctx, val) ||
        JSValueIsNull(ctx, val)) {
        return NULL;
    }

    if (JSValueIsBoolean(ctx, val)) {
        return AbstractPlugin::newBoolean(env, plugInObj, JSValueToBoolean(ctx, val));
    }

    if (JSValueIsNumber(ctx, val)) {
        return AbstractPlugin::newDouble(env, plugInObj, JSValueToNumber(ctx, val, NULL));
    }

    if (JSValueIsString(ctx, val)) {
        // Unclear whether we could use JSValueToObject and cast the result to JSStringRef
        return jsStringToJString(env, JSValueToStringCopy(ctx, val, NULL));
    }

    if (JSValueIsObject(ctx, val)) {
        // Otherwise, we need to wrap this up to send a remote reference over to Java
        // First, see whether this is one of ours
        if (JSValueIsObjectOfClass(ctx, val, JavaJSObject_GetClass())) {
            return JavaJSObject_GetTargetObject(JSValueToObject(ctx, val, NULL));
        }

        return AbstractPlugin::wrapOrUnwrapScriptingObject(env, plugInObj, (jlong) val);
    }

    // Don't know how to deal with this object
    assert("Unknown kind of JavaScript object");
    return NULL;
}

JNIEXPORT jlong JNICALL Java_sun_plugin2_main_server_SafariPlugin_allocateJavaObject
  (JNIEnv *env, jobject parentPlugin, jlong webPlugInContainer, jobject targetObject)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    return (jlong) JavaJSObject_Allocate(ctx, env, parentPlugin, targetObject);
}

JNIEXPORT jlong JNICALL Java_sun_plugin2_main_server_SafariPlugin_allocateJavaObjectForNameSpace
  (JNIEnv *env, jobject parentPlugin, jlong webPlugInContainer, jstring nameSpace)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    const char* chars = env->GetStringUTFChars(nameSpace, NULL);
    JSValueRef res = JavaJSObject_AllocateForNameSpace(ctx, env, parentPlugin, chars);
    env->ReleaseStringUTFChars(nameSpace, chars);
    return (jlong) res;
}

JNIEXPORT void JNICALL Java_sun_plugin2_main_server_SafariPlugin_fillInExceptionInfo0
  (JNIEnv *env, jobject unused, jlong webPlugInContainer, jlong exceptionVal, jstring message)
{
    NSObject* container = (NSObject*) webPlugInContainer;
    WebFrame* frame = [container webFrame];
    JSContextRef ctx = [frame globalContext];
    JSStringRef str = jstringToJSString(env, message);
    JSValueRef* exception = (JSValueRef*) exceptionVal;
    JavaJSObject_ThrowException(ctx, str, exception);
}
