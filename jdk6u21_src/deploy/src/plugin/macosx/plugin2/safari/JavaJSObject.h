/*
 * @(#)JavaJSObject.h	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <JavaScriptCore/JSObjectRef.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Public entry points */
void        JavaJSObject_GlobalInitialize();
JSValueRef  JavaJSObject_Allocate(JSContextRef ctx, JNIEnv* env, jobject parentJavaPlugin, jobject targetObject);
JSValueRef  JavaJSObject_AllocateForNameSpace(JSContextRef ctx, JNIEnv* env, jobject parentJavaPlugin, const char* nameSpace);
JSClassRef  JavaJSObject_GetClass();
/* This assumes a type check of the JSObjectRef has already been done by the caller */
jobject     JavaJSObject_GetTargetObject(JSObjectRef object);

/* This is only a helper method */
void        JavaJSObject_ThrowException(JSContextRef ctx, JSStringRef exceptionString, JSValueRef* exception);

#ifdef __cplusplus
}
#endif
