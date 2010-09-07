/*
 * @(#)remotejni.h	1.15 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* 
 * Copied from the JDK and modified (on the checkout date above)
 * The modifications are the following:
 *  - the basic java types are removed - jni.h is included
 *  - the name of the function pointer struct is changed to RemoteJNI
 *  - a RemoteJNINativeMethod struct is defined
 *  - the invocation interface is removed
 *  - environmentInfo is added to the end of the remotejni struct
 */

/*
 * We used part of Netscape's Java Runtime Interface (JRI) as the starting
 * point of our design and implementation.
 */

/******************************************************************************
 * Java Runtime Interface
 * Copyright (c) 1996 Netscape Communications Corporation. All rights reserved.
 *****************************************************************************/

/*
 * Modified by: Robert Szewczyk, Benedict Gomes
 */

/*
 * Specialization for our remote process version of JNI
 */
#ifndef REMOTEJNI_H
#define REMOTEJNI_H
#define JNI_H

#include "commonhdr.h"
#include "jni.h"
#include "ISecureEnv.h"
#include "ISecurityContext.h"

class JavaPluginFactory5;

/*XXXBegin for remote */
/* Information for our particular implementation of the JNIEnv */
struct environmentInfo_ {

    /* An index used to refer to this env, its position in the 
       env table */
    int env_index;

    /* A pointer to a plugin factory */
    JavaPluginFactory5* factory;

    /* The pipe associated with this env */
    void* pipe;

    /* Reference count for this env */
    int refcnt;

  /* Recursive calling depth. Incremented for each call back from Java */
   int call_depth;

  /* Pointer to the proxy env held on the browser side */
  JNIEnv* proxy_env;

};


#ifdef __cplusplus
extern "C" {
#endif

/*
 * used in ReleaseScalarArrayElements
 */
  
#define REMOTEJNI_COMMIT 1
#define REMOTEJNI_ABORT 2

/*
 * used in RegisterNatives to describe native method name, signature,
 * and function pointer.
 */

typedef struct {
    char *name;
    char *signature;
    void *fnPtr;
} RemoteJNINativeMethod;

/*
 * RemoteJNI Native Method Interface.
 */

struct RemoteJNINativeInterface_;

struct RemoteJNIEnv_;

#ifdef __cplusplus
typedef RemoteJNIEnv_ RemoteJNIEnv;
#else
typedef const struct RemoteJNINativeInterface_ *RemoteJNIEnv;
#endif

/* This must be changed. The _jmethodID type should be the raw ID returned
   by the JNI call. There should be an additional hash table between
   methodIDs and signatures. Possibly on a per-class basis so that signatures
   may later be freed by further JNI calls - BG */
struct _jmethodID {
    void *java_method;
    char *signature;
};

/*XXXEnd for remote */

struct RemoteJNINativeInterface_ {
    void *reserved0;
    void *reserved1;
    void *reserved2;

    void *reserved3;
    jint ( *GetVersion)(RemoteJNIEnv *env);

    jclass ( *DefineClass)
      (RemoteJNIEnv *env, const char *name, jobject loader, const jbyte *buf, 
       jsize len);
    jclass ( *FindClass)
      (RemoteJNIEnv *env, const char *name);

    jmethodID ( *FromReflectedMethod)
      (RemoteJNIEnv *env, jobject method);
    jfieldID ( *FromReflectedField)
      (RemoteJNIEnv *env, jobject field);

    jobject ( *ToReflectedMethod)
      (RemoteJNIEnv *env, jclass cls, jmethodID methodID);

    jclass ( *GetSuperclass)
      (RemoteJNIEnv *env, jclass sub);
    jboolean ( *IsAssignableFrom)
      (RemoteJNIEnv *env, jclass sub, jclass sup);

    jobject ( *ToReflectedField)
      (RemoteJNIEnv *env, jclass cls, jfieldID fieldID);

    jint ( *Throw)
      (RemoteJNIEnv *env, jthrowable obj);
    jint ( *ThrowNew)
      (RemoteJNIEnv *env, jclass clazz, const char *msg);
    jthrowable ( *ExceptionOccurred)
      (RemoteJNIEnv *env);
    void ( *ExceptionDescribe)
      (RemoteJNIEnv *env);
    void ( *ExceptionClear)
      (RemoteJNIEnv *env);
    void ( *FatalError)
      (RemoteJNIEnv *env, const char *msg);

    jint ( *PushLocalFrame)
      (RemoteJNIEnv *env, jint capacity);
    jobject ( *PopLocalFrame)
      (RemoteJNIEnv *env, jobject result);

    jobject ( *NewGlobalRef)
      (RemoteJNIEnv *env, jobject lobj);
    void ( *DeleteGlobalRef)
      (RemoteJNIEnv *env, jobject gref);
    void ( *DeleteLocalRef)
      (RemoteJNIEnv *env, jobject obj);
    jboolean ( *IsSameObject)
      (RemoteJNIEnv *env, jobject obj1, jobject obj2);
    jobject ( *NewLocalRef)
      (RemoteJNIEnv *env, jobject ref);
    jint ( *EnsureLocalCapacity)
      (RemoteJNIEnv *env, jint capacity);

    jobject ( *AllocObject)
      (RemoteJNIEnv *env, jclass clazz);
    jobject ( *NewObject)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jobject ( *NewObjectV)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jobject ( *NewObjectA)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jclass ( *GetObjectClass)
      (RemoteJNIEnv *env, jobject obj);
    jboolean ( *IsInstanceOf)
      (RemoteJNIEnv *env, jobject obj, jclass clazz);

    jmethodID ( *GetMethodID)
      (RemoteJNIEnv *env, jclass clazz, const char *name, const char *sig);

    jobject ( *CallObjectMethod)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...);
    jobject ( *CallObjectMethodV)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jobject ( *CallObjectMethodA)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, jvalue * args);

    jboolean ( *CallBooleanMethod)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...);
    jboolean ( *CallBooleanMethodV)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jboolean ( *CallBooleanMethodA)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, jvalue * args);

    jbyte ( *CallByteMethod)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...);
    jbyte ( *CallByteMethodV)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jbyte ( *CallByteMethodA)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    jchar ( *CallCharMethod)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...);
    jchar ( *CallCharMethodV)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jchar ( *CallCharMethodA)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    jshort ( *CallShortMethod)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...);
    jshort ( *CallShortMethodV)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jshort ( *CallShortMethodA)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    jint ( *CallIntMethod)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...);
    jint ( *CallIntMethodV)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jint ( *CallIntMethodA)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    jlong ( *CallLongMethod)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...);
    jlong ( *CallLongMethodV)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jlong ( *CallLongMethodA)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    jfloat ( *CallFloatMethod)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...);
    jfloat ( *CallFloatMethodV)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jfloat ( *CallFloatMethodA)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    jdouble ( *CallDoubleMethod)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...);
    jdouble ( *CallDoubleMethodV)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jdouble ( *CallDoubleMethodA)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, jvalue *args);

    void ( *CallVoidMethod)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...);
    void ( *CallVoidMethodV)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    void ( *CallVoidMethodA)
      (RemoteJNIEnv *env, jobject obj, jmethodID methodID, jvalue * args);

    jobject ( *CallNonvirtualObjectMethod)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jobject ( *CallNonvirtualObjectMethodV)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, 
       va_list args);
    jobject ( *CallNonvirtualObjectMethodA)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, 
       jvalue * args);

    jboolean ( *CallNonvirtualBooleanMethod)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jboolean ( *CallNonvirtualBooleanMethodV)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jboolean ( *CallNonvirtualBooleanMethodA)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue * args);

    jbyte ( *CallNonvirtualByteMethod)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jbyte ( *CallNonvirtualByteMethodV)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jbyte ( *CallNonvirtualByteMethodA)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, 
       jvalue *args);

    jchar ( *CallNonvirtualCharMethod)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jchar ( *CallNonvirtualCharMethodV)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jchar ( *CallNonvirtualCharMethodA)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue *args);

    jshort ( *CallNonvirtualShortMethod)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jshort ( *CallNonvirtualShortMethodV)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jshort ( *CallNonvirtualShortMethodA)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue *args);

    jint ( *CallNonvirtualIntMethod)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jint ( *CallNonvirtualIntMethodV)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jint ( *CallNonvirtualIntMethodA)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue *args);

    jlong ( *CallNonvirtualLongMethod)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jlong ( *CallNonvirtualLongMethodV)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jlong ( *CallNonvirtualLongMethodA)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, 
       jvalue *args);

    jfloat ( *CallNonvirtualFloatMethod)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jfloat ( *CallNonvirtualFloatMethodV)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jfloat ( *CallNonvirtualFloatMethodA)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue *args);

    jdouble ( *CallNonvirtualDoubleMethod)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jdouble ( *CallNonvirtualDoubleMethodV)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jdouble ( *CallNonvirtualDoubleMethodA)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue *args);

    void ( *CallNonvirtualVoidMethod)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    void ( *CallNonvirtualVoidMethodV)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    void ( *CallNonvirtualVoidMethodA)
      (RemoteJNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       jvalue * args);

    jfieldID ( *GetFieldID)
      (RemoteJNIEnv *env, jclass clazz, const char *name, const char *sig);

    jobject ( *GetObjectField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID);
    jboolean ( *GetBooleanField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID);
    jbyte ( *GetByteField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID);
    jchar ( *GetCharField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID);
    jshort ( *GetShortField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID);
    jint ( *GetIntField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID);
    jlong ( *GetLongField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID);
    jfloat ( *GetFloatField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID);
    jdouble ( *GetDoubleField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID);

    void ( *SetObjectField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID, jobject val);
    void ( *SetBooleanField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID, jboolean val);
    void ( *SetByteField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID, jbyte val);
    void ( *SetCharField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID, jchar val);
    void ( *SetShortField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID, jshort val);
    void ( *SetIntField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID, jint val);
    void ( *SetLongField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID, jlong val);
    void ( *SetFloatField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID, jfloat val);
    void ( *SetDoubleField)
      (RemoteJNIEnv *env, jobject obj, jfieldID fieldID, jdouble val);

    jmethodID ( *GetStaticMethodID)
      (RemoteJNIEnv *env, jclass clazz, const char *name, const char *sig);

    jobject ( *CallStaticObjectMethod)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jobject ( *CallStaticObjectMethodV)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jobject ( *CallStaticObjectMethodA)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jboolean ( *CallStaticBooleanMethod)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jboolean ( *CallStaticBooleanMethodV)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jboolean ( *CallStaticBooleanMethodA)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jbyte ( *CallStaticByteMethod)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jbyte ( *CallStaticByteMethodV)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jbyte ( *CallStaticByteMethodA)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jchar ( *CallStaticCharMethod)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jchar ( *CallStaticCharMethodV)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jchar ( *CallStaticCharMethodA)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jshort ( *CallStaticShortMethod)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jshort ( *CallStaticShortMethodV)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jshort ( *CallStaticShortMethodA)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jint ( *CallStaticIntMethod)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jint ( *CallStaticIntMethodV)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jint ( *CallStaticIntMethodA)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jlong ( *CallStaticLongMethod)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jlong ( *CallStaticLongMethodV)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jlong ( *CallStaticLongMethodA)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jfloat ( *CallStaticFloatMethod)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jfloat ( *CallStaticFloatMethodV)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jfloat ( *CallStaticFloatMethodA)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    jdouble ( *CallStaticDoubleMethod)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jdouble ( *CallStaticDoubleMethodV)
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jdouble ( *CallStaticDoubleMethodA)       
      (RemoteJNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args);

    void ( *CallStaticVoidMethod)
      (RemoteJNIEnv *env, jclass cls, jmethodID methodID, ...);
    void ( *CallStaticVoidMethodV)
      (RemoteJNIEnv *env, jclass cls, jmethodID methodID, va_list args);
    void ( *CallStaticVoidMethodA)
      (RemoteJNIEnv *env, jclass cls, jmethodID methodID, jvalue * args);

    jfieldID ( *GetStaticFieldID)
      (RemoteJNIEnv *env, jclass clazz, const char *name, const char *sig);
    jobject ( *GetStaticObjectField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID);
    jboolean ( *GetStaticBooleanField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID);
    jbyte ( *GetStaticByteField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID);
    jchar ( *GetStaticCharField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID);
    jshort ( *GetStaticShortField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID);
    jint ( *GetStaticIntField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID);
    jlong ( *GetStaticLongField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID);
    jfloat ( *GetStaticFloatField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID);
    jdouble ( *GetStaticDoubleField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID);

    void ( *SetStaticObjectField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID, jobject value);
    void ( *SetStaticBooleanField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID, jboolean value);
    void ( *SetStaticByteField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID, jbyte value);
    void ( *SetStaticCharField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID, jchar value);
    void ( *SetStaticShortField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID, jshort value);
    void ( *SetStaticIntField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID, jint value);
    void ( *SetStaticLongField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID, jlong value);
    void ( *SetStaticFloatField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID, jfloat value);
    void ( *SetStaticDoubleField)
      (RemoteJNIEnv *env, jclass clazz, jfieldID fieldID, jdouble value);

    jstring ( *NewString)
      (RemoteJNIEnv *env, const jchar *unicode, jsize len);
    jsize ( *GetStringLength)
      (RemoteJNIEnv *env, jstring str);
    const jchar *( *GetStringChars)
      (RemoteJNIEnv *env, jstring str, jboolean *isCopy);
    void ( *ReleaseStringChars)
      (RemoteJNIEnv *env, jstring str, const jchar *chars);
  
    jstring ( *NewStringUTF)
      (RemoteJNIEnv *env, const char *utf);
    jsize ( *GetStringUTFLength)
      (RemoteJNIEnv *env, jstring str);
    const char* ( *GetStringUTFChars)
      (RemoteJNIEnv *env, jstring str, jboolean *isCopy);
    void ( *ReleaseStringUTFChars)
      (RemoteJNIEnv *env, jstring str, const char* chars);
  

    jsize ( *GetArrayLength)
      (RemoteJNIEnv *env, jarray array);

    jobjectArray ( *NewObjectArray)
      (RemoteJNIEnv *env, jsize len, jclass clazz, jobject init);
    jobject ( *GetObjectArrayElement)
      (RemoteJNIEnv *env, jobjectArray array, jsize index);
    void ( *SetObjectArrayElement)
      (RemoteJNIEnv *env, jobjectArray array, jsize index, jobject val);

    jbooleanArray ( *NewBooleanArray)
      (RemoteJNIEnv *env, jsize len);
    jbyteArray ( *NewByteArray)
      (RemoteJNIEnv *env, jsize len);
    jcharArray ( *NewCharArray)
      (RemoteJNIEnv *env, jsize len);
    jshortArray ( *NewShortArray)
      (RemoteJNIEnv *env, jsize len);
    jintArray ( *NewIntArray)
      (RemoteJNIEnv *env, jsize len);
    jlongArray ( *NewLongArray)
      (RemoteJNIEnv *env, jsize len);
    jfloatArray ( *NewFloatArray)
      (RemoteJNIEnv *env, jsize len);
    jdoubleArray ( *NewDoubleArray)
      (RemoteJNIEnv *env, jsize len);

    jboolean * ( *GetBooleanArrayElements)
      (RemoteJNIEnv *env, jbooleanArray array, jboolean *isCopy);
    jbyte * ( *GetByteArrayElements)
      (RemoteJNIEnv *env, jbyteArray array, jboolean *isCopy);
    jchar * ( *GetCharArrayElements)
      (RemoteJNIEnv *env, jcharArray array, jboolean *isCopy);
    jshort * ( *GetShortArrayElements)
      (RemoteJNIEnv *env, jshortArray array, jboolean *isCopy);
    jint * ( *GetIntArrayElements)
      (RemoteJNIEnv *env, jintArray array, jboolean *isCopy);
    jlong * ( *GetLongArrayElements)
      (RemoteJNIEnv *env, jlongArray array, jboolean *isCopy);
    jfloat * ( *GetFloatArrayElements)
      (RemoteJNIEnv *env, jfloatArray array, jboolean *isCopy);
    jdouble * ( *GetDoubleArrayElements)
      (RemoteJNIEnv *env, jdoubleArray array, jboolean *isCopy);

    void ( *ReleaseBooleanArrayElements)
      (RemoteJNIEnv *env, jbooleanArray array, jboolean *elems, jint mode);
    void ( *ReleaseByteArrayElements)
      (RemoteJNIEnv *env, jbyteArray array, jbyte *elems, jint mode);
    void ( *ReleaseCharArrayElements)
      (RemoteJNIEnv *env, jcharArray array, jchar *elems, jint mode);
    void ( *ReleaseShortArrayElements)
      (RemoteJNIEnv *env, jshortArray array, jshort *elems, jint mode);
    void ( *ReleaseIntArrayElements)
      (RemoteJNIEnv *env, jintArray array, jint *elems, jint mode);
    void ( *ReleaseLongArrayElements)
      (RemoteJNIEnv *env, jlongArray array, jlong *elems, jint mode);
    void ( *ReleaseFloatArrayElements)
      (RemoteJNIEnv *env, jfloatArray array, jfloat *elems, jint mode);
    void ( *ReleaseDoubleArrayElements)
      (RemoteJNIEnv *env, jdoubleArray array, jdouble *elems, jint mode);

    void ( *GetBooleanArrayRegion)
      (RemoteJNIEnv *env, jbooleanArray array, jsize start, jsize l, jboolean *buf);
    void ( *GetByteArrayRegion)
      (RemoteJNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf);
    void ( *GetCharArrayRegion)
      (RemoteJNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf);
    void ( *GetShortArrayRegion)
      (RemoteJNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf);
    void ( *GetIntArrayRegion)
      (RemoteJNIEnv *env, jintArray array, jsize start, jsize len, jint *buf);
    void ( *GetLongArrayRegion)
      (RemoteJNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf);
    void ( *GetFloatArrayRegion)
      (RemoteJNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf);
    void ( *GetDoubleArrayRegion)
      (RemoteJNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf);

    void ( *SetBooleanArrayRegion)
      (RemoteJNIEnv *env, jbooleanArray array, jsize start, jsize l, jboolean *buf);
    void ( *SetByteArrayRegion)
      (RemoteJNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf);
    void ( *SetCharArrayRegion)
      (RemoteJNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf);
    void ( *SetShortArrayRegion)
      (RemoteJNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf);
    void ( *SetIntArrayRegion)
      (RemoteJNIEnv *env, jintArray array, jsize start, jsize len, jint *buf);
    void ( *SetLongArrayRegion)
      (RemoteJNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf);
    void ( *SetFloatArrayRegion)
      (RemoteJNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf);
    void ( *SetDoubleArrayRegion)
      (RemoteJNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf);

    jint ( *RegisterNatives)
      (RemoteJNIEnv *env, jclass clazz, const RemoteJNINativeMethod *methods, 
       jint nMethods);
    jint ( *UnregisterNatives)
      (RemoteJNIEnv *env, jclass clazz);

    jint ( *MonitorEnter)
      (RemoteJNIEnv *env, jobject obj);
    jint ( *MonitorExit)
      (RemoteJNIEnv *env, jobject obj);
 
    jint ( *GetJavaVM)
      (RemoteJNIEnv *env, void **vm);

    /* Secure JNI Functions */
    int ( *SecureNewObject)(RemoteJNIEnv* env, jclass clazz,
			     jmethodID methodID,
			     jvalue* args, jobject* result, 
			     ISecurityContext* ctx);

    int ( *SecureCallMethod)(RemoteJNIEnv* env, 
			     jd_jni_type type,
			     jobject obj,
			     jmethodID methodID,
			     jvalue* args, 
			     jvalue* result, 
			     ISecurityContext* ctx);

    int ( *SecureCallNonvirtualMethod)(RemoteJNIEnv* env,
					jd_jni_type type,
					jobject obj,
					jclass clazz,
					jmethodID methodID,
					jvalue *args,
					jvalue* result,
					ISecurityContext* ctx);
    
    int ( *SecureGetField)(RemoteJNIEnv* env, 
			    jd_jni_type type,
			    jobject obj,
			    jfieldID fieldID,
			    jvalue* result,
			    ISecurityContext* ctx);

    int ( *SecureSetField)(RemoteJNIEnv* env, 
			    jd_jni_type type,
			    jobject obj,
			    jfieldID fieldID,
			    jvalue val,
			    ISecurityContext* ctx);


    int ( *SecureCallStaticMethod)(RemoteJNIEnv* env,
				    jd_jni_type type,
				    jclass clazz,
				    jmethodID methodID,
				    jvalue* args,
				    jvalue* result,
				    ISecurityContext* ctx);

    int (*SecureGetStaticField)(RemoteJNIEnv* env,
				 jd_jni_type type,
				 jclass clazz,
				 jfieldID fieldID,
				 jvalue* result,
				 ISecurityContext* ctx);

    int (*SecureSetStaticField)(RemoteJNIEnv* env,
				 jd_jni_type type,
				 jclass clazz,
				 jfieldID fieldID,
				 jvalue val,
				 ISecurityContext* ctx);
    /* Return 0 if the action is permitted. The target is
       current AllJavaPermission or AllJavaScriptPermission.
       The action is essentially ignored.
       */
    int (*CSecurityContextImplies)(RemoteJNIEnv* env,
				   jobject context,
				   const char* target, 
				   const char* action);

      /* 1.2 functions - not implemented */
    void ( *GetStringRegion)
      (RemoteJNIEnv *env, jstring str, jsize start, jsize len, jchar *buf);
    void ( *GetStringUTFRegion)
      (RemoteJNIEnv *env, jstring str, jsize start, jsize len, char *buf);

    void * ( *GetPrimitiveArrayCritical)
      (RemoteJNIEnv *env, jarray array, jboolean *isCopy);
    void ( *ReleasePrimitiveArrayCritical)
      (RemoteJNIEnv *env, jarray array, void *carray, jint mode);

    const jchar * ( *GetStringCritical)
      (RemoteJNIEnv *env, jstring string, jboolean *isCopy);
    void ( *ReleaseStringCritical)
      (RemoteJNIEnv *env, jstring string, const jchar *cstring);

    jweak ( *NewWeakGlobalRef)
       (RemoteJNIEnv *env, jobject obj);
    void ( *DeleteWeakGlobalRef)
       (RemoteJNIEnv *env, jweak ref);

    jboolean ( *ExceptionCheck)
       (RemoteJNIEnv *env);


    /*XXXBegin for remote */
    struct environmentInfo_ * environmentInfo;
    /*XXXEnd for remote */
};

/*
 * We use inlined functions for C++ so that programmers can write:
 * 
 *    env->FindClass("java/lang/String")
 *
 * in C++ rather than:
 *
 *    (*env)->FindClass(env, "java/lang/String")
 *
 * in C.
 */

struct RemoteJNIEnv_ {

    const struct RemoteJNINativeInterface_ *functions;
#ifdef __cplusplus

    jint GetVersion() {
        return functions->GetVersion(this);
    }
    jclass DefineClass(const char *name, jobject loader, const jbyte *buf,
		       jsize len) {
        return functions->DefineClass(this, name, loader, buf, len);
    }
    jclass FindClass(const char *name) {
        return functions->FindClass(this, name);
    }
    jmethodID FromReflectedMethod(jobject method) {
        return functions->FromReflectedMethod(this,method);
    }
    jfieldID FromReflectedField(jobject field) {
        return functions->FromReflectedField(this,field);
    }

    jobject ToReflectedMethod(jclass cls, jmethodID methodID) {
        return functions->ToReflectedMethod(this, cls, methodID);
    }

    jclass GetSuperclass(jclass sub) {
        return functions->GetSuperclass(this, sub);
    }
    jboolean IsAssignableFrom(jclass sub, jclass sup) {
        return functions->IsAssignableFrom(this, sub, sup);
    }

    jobject ToReflectedField(jclass cls, jfieldID fieldID) {
        return functions->ToReflectedField(this,cls, fieldID);
    }

    jint Throw(jthrowable obj) {
        return functions->Throw(this, obj);
    }    
    jint ThrowNew(jclass clazz, const char *msg) {
        return functions->ThrowNew(this, clazz, msg);
    }
    jthrowable ExceptionOccurred() {
        return functions->ExceptionOccurred(this);
    }
    void ExceptionDescribe() {
        functions->ExceptionDescribe(this);
    }
    void ExceptionClear() {
        functions->ExceptionClear(this);
    }
    void FatalError(const char *msg) {
        functions->FatalError(this, msg);
    }

    jint PushLocalFrame(jint capacity) {
        return functions->PushLocalFrame(this,capacity);
    }
    jobject PopLocalFrame(jobject result) {
        return functions->PopLocalFrame(this,result);
    }

    jobject NewGlobalRef(jobject lobj) {
        return functions->NewGlobalRef(this,lobj);
    }
    void DeleteGlobalRef(jobject gref) {
        functions->DeleteGlobalRef(this,gref);
    }
    void DeleteLocalRef(jobject obj) {
        functions->DeleteLocalRef(this, obj);
    }

    jboolean IsSameObject(jobject obj1, jobject obj2) {
        return functions->IsSameObject(this,obj1,obj2);
    }

    jobject NewLocalRef(jobject ref) {
        return functions->NewLocalRef(this,ref);
    }
    jint EnsureLocalCapacity(jint capacity) {
        return functions->EnsureLocalCapacity(this,capacity);
    }

    jobject AllocObject(jclass clazz) {
        return functions->AllocObject(this,clazz);
    }
    jobject NewObject(jclass clazz, jmethodID methodID, ...) {
        va_list args;
	jobject result;
	va_start(args, methodID);
        result = functions->NewObjectV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jobject NewObjectV(jclass clazz, jmethodID methodID, 
		       va_list args) {
        return functions->NewObjectV(this,clazz,methodID,args);
    }
    jobject NewObjectA(jclass clazz, jmethodID methodID, 
		       jvalue *args) {
        return functions->NewObjectA(this,clazz,methodID,args);
    }

    jclass GetObjectClass(jobject obj) {
        return functions->GetObjectClass(this,obj);
    }
    jboolean IsInstanceOf(jobject obj, jclass clazz) {
        return functions->IsInstanceOf(this,obj,clazz);
    }

    jmethodID GetMethodID(jclass clazz, const char *name, 
			  const char *sig) {
        return functions->GetMethodID(this,clazz,name,sig);
    }

    jobject CallObjectMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jobject result;
	va_start(args,methodID);
	result = functions->CallObjectMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jobject CallObjectMethodV(jobject obj, jmethodID methodID, 
			va_list args) {
        return functions->CallObjectMethodV(this,obj,methodID,args);
    }
    jobject CallObjectMethodA(jobject obj, jmethodID methodID, 
			jvalue * args) {
        return functions->CallObjectMethodA(this,obj,methodID,args);
    }

    jboolean CallBooleanMethod(jobject obj, 
			       jmethodID methodID, ...) {
        va_list args;
	jboolean result;
	va_start(args,methodID);
	result = functions->CallBooleanMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jboolean CallBooleanMethodV(jobject obj, jmethodID methodID, 
				va_list args) {
        return functions->CallBooleanMethodV(this,obj,methodID,args);
    }
    jboolean CallBooleanMethodA(jobject obj, jmethodID methodID, 
				jvalue * args) {
        return functions->CallBooleanMethodA(this,obj,methodID, args);
    }

    jbyte CallByteMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jbyte result;
	va_start(args,methodID);
	result = functions->CallByteMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jbyte CallByteMethodV(jobject obj, jmethodID methodID, 
			  va_list args) {
        return functions->CallByteMethodV(this,obj,methodID,args);
    }
    jbyte CallByteMethodA(jobject obj, jmethodID methodID, 
			  jvalue * args) {
        return functions->CallByteMethodA(this,obj,methodID,args);
    }

    jchar CallCharMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jchar result;
	va_start(args,methodID);
	result = functions->CallCharMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jchar CallCharMethodV(jobject obj, jmethodID methodID, 
			  va_list args) {
        return functions->CallCharMethodV(this,obj,methodID,args);
    }
    jchar CallCharMethodA(jobject obj, jmethodID methodID, 
			  jvalue * args) {
        return functions->CallCharMethodA(this,obj,methodID,args);
    }

    jshort CallShortMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jshort result;
	va_start(args,methodID);
	result = functions->CallShortMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jshort CallShortMethodV(jobject obj, jmethodID methodID, 
			    va_list args) {
        return functions->CallShortMethodV(this,obj,methodID,args);
    }
    jshort CallShortMethodA(jobject obj, jmethodID methodID, 
			    jvalue * args) {
        return functions->CallShortMethodA(this,obj,methodID,args);
    }

    jint CallIntMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jint result;
	va_start(args,methodID);
	result = functions->CallIntMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jint CallIntMethodV(jobject obj, jmethodID methodID, 
			va_list args) {
        return functions->CallIntMethodV(this,obj,methodID,args);
    }
    jint CallIntMethodA(jobject obj, jmethodID methodID, 
			jvalue * args) {
        return functions->CallIntMethodA(this,obj,methodID,args);
    }

    jlong CallLongMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jlong result;
	va_start(args,methodID);
	result = functions->CallLongMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jlong CallLongMethodV(jobject obj, jmethodID methodID, 
			  va_list args) {
        return functions->CallLongMethodV(this,obj,methodID,args);
    }
    jlong CallLongMethodA(jobject obj, jmethodID methodID, 
			  jvalue * args) {
        return functions->CallLongMethodA(this,obj,methodID,args);
    }

    jfloat CallFloatMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jfloat result;
	va_start(args,methodID);
	result = functions->CallFloatMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jfloat CallFloatMethodV(jobject obj, jmethodID methodID, 
			    va_list args) {
        return functions->CallFloatMethodV(this,obj,methodID,args);
    }
    jfloat CallFloatMethodA(jobject obj, jmethodID methodID, 
			    jvalue * args) {
        return functions->CallFloatMethodA(this,obj,methodID,args);
    }

    jdouble CallDoubleMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	jdouble result;
	va_start(args,methodID);
	result = functions->CallDoubleMethodV(this,obj,methodID,args);
	va_end(args);
	return result;
    }
    jdouble CallDoubleMethodV(jobject obj, jmethodID methodID, 
			va_list args) {
        return functions->CallDoubleMethodV(this,obj,methodID,args);
    }
    jdouble CallDoubleMethodA(jobject obj, jmethodID methodID, 
			jvalue * args) {
        return functions->CallDoubleMethodA(this,obj,methodID,args);
    }

    void CallVoidMethod(jobject obj, jmethodID methodID, ...) {
        va_list args;
	va_start(args,methodID);
	functions->CallVoidMethodV(this,obj,methodID,args);
	va_end(args);
    }
    void CallVoidMethodV(jobject obj, jmethodID methodID, 
			 va_list args) {
        functions->CallVoidMethodV(this,obj,methodID,args);
    }
    void CallVoidMethodA(jobject obj, jmethodID methodID, 
			 jvalue * args) {
        functions->CallVoidMethodA(this,obj,methodID,args);
    }

    jobject CallNonvirtualObjectMethod(jobject obj, jclass clazz, 
				       jmethodID methodID, ...) {
        va_list args;
	jobject result;
	va_start(args,methodID);
	result = functions->CallNonvirtualObjectMethodV(this,obj,clazz,
							methodID,args);
	va_end(args);
	return result;
    }
    jobject CallNonvirtualObjectMethodV(jobject obj, jclass clazz, 
					jmethodID methodID, va_list args) {
        return functions->CallNonvirtualObjectMethodV(this,obj,clazz,
						      methodID,args);
    }
    jobject CallNonvirtualObjectMethodA(jobject obj, jclass clazz, 
					jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualObjectMethodA(this,obj,clazz,
						      methodID,args);
    }

    jboolean CallNonvirtualBooleanMethod(jobject obj, jclass clazz, 
					 jmethodID methodID, ...) {
        va_list args;
	jboolean result;
	va_start(args,methodID);
	result = functions->CallNonvirtualBooleanMethodV(this,obj,clazz,
							 methodID,args);
	va_end(args);
	return result;
    }
    jboolean CallNonvirtualBooleanMethodV(jobject obj, jclass clazz, 
					  jmethodID methodID, va_list args) {
        return functions->CallNonvirtualBooleanMethodV(this,obj,clazz,
						       methodID,args);
    }
    jboolean CallNonvirtualBooleanMethodA(jobject obj, jclass clazz, 
					  jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualBooleanMethodA(this,obj,clazz,
						       methodID, args);
    }

    jbyte CallNonvirtualByteMethod(jobject obj, jclass clazz, 
				   jmethodID methodID, ...) {
        va_list args;
	jbyte result;
	va_start(args,methodID);
	result = functions->CallNonvirtualByteMethodV(this,obj,clazz,
						      methodID,args);
	va_end(args);
	return result;
    }
    jbyte CallNonvirtualByteMethodV(jobject obj, jclass clazz, 
				    jmethodID methodID, va_list args) {
        return functions->CallNonvirtualByteMethodV(this,obj,clazz,
						    methodID,args);
    }
    jbyte CallNonvirtualByteMethodA(jobject obj, jclass clazz, 
				    jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualByteMethodA(this,obj,clazz,
						    methodID,args);
    }

    jchar CallNonvirtualCharMethod(jobject obj, jclass clazz, 
				   jmethodID methodID, ...) {
        va_list args;
	jchar result;
	va_start(args,methodID);
	result = functions->CallNonvirtualCharMethodV(this,obj,clazz,
						      methodID,args);
	va_end(args);
	return result;
    }
    jchar CallNonvirtualCharMethodV(jobject obj, jclass clazz, 
				    jmethodID methodID, va_list args) {
        return functions->CallNonvirtualCharMethodV(this,obj,clazz,
						    methodID,args);
    }
    jchar CallNonvirtualCharMethodA(jobject obj, jclass clazz, 
				    jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualCharMethodA(this,obj,clazz,
						    methodID,args);
    }

    jshort CallNonvirtualShortMethod(jobject obj, jclass clazz, 
				     jmethodID methodID, ...) {
        va_list args;
	jshort result;
	va_start(args,methodID);
	result = functions->CallNonvirtualShortMethodV(this,obj,clazz,
						       methodID,args);
	va_end(args);
	return result;
    }
    jshort CallNonvirtualShortMethodV(jobject obj, jclass clazz, 
				      jmethodID methodID, va_list args) {
        return functions->CallNonvirtualShortMethodV(this,obj,clazz,
						     methodID,args);
    }
    jshort CallNonvirtualShortMethodA(jobject obj, jclass clazz,
				      jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualShortMethodA(this,obj,clazz,
						     methodID,args);
    }

    jint CallNonvirtualIntMethod(jobject obj, jclass clazz, 
				 jmethodID methodID, ...) {
        va_list args;
	jint result;
	va_start(args,methodID);
	result = functions->CallNonvirtualIntMethodV(this,obj,clazz,
						     methodID,args);
	va_end(args);
	return result;
    }
    jint CallNonvirtualIntMethodV(jobject obj, jclass clazz, 
				  jmethodID methodID, va_list args) {
        return functions->CallNonvirtualIntMethodV(this,obj,clazz,
						   methodID,args);
    }
    jint CallNonvirtualIntMethodA(jobject obj, jclass clazz, 
				  jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualIntMethodA(this,obj,clazz,
						   methodID,args);
    }

    jlong CallNonvirtualLongMethod(jobject obj, jclass clazz,
				   jmethodID methodID, ...) {
        va_list args;
	jlong result;
	va_start(args,methodID);
	result = functions->CallNonvirtualLongMethodV(this,obj,clazz,
						      methodID,args);
	va_end(args);
	return result;
    }
    jlong CallNonvirtualLongMethodV(jobject obj, jclass clazz,
				    jmethodID methodID, va_list args) {
        return functions->CallNonvirtualLongMethodV(this,obj,clazz,
						    methodID,args);
    }
    jlong CallNonvirtualLongMethodA(jobject obj, jclass clazz, 
				    jmethodID methodID, jvalue * args) {
        return functions->CallNonvirtualLongMethodA(this,obj,clazz,
						    methodID,args);
    }

    jfloat CallNonvirtualFloatMethod(jobject obj, jclass clazz, 
				     jmethodID methodID, ...) {
        va_list args;
	jfloat result;
	va_start(args,methodID);
	result = functions->CallNonvirtualFloatMethodV(this,obj,clazz,
						       methodID,args);
	va_end(args);
	return result;
    }
    jfloat CallNonvirtualFloatMethodV(jobject obj, jclass clazz,
				      jmethodID methodID, 
				      va_list args) {
        return functions->CallNonvirtualFloatMethodV(this,obj,clazz,
						     methodID,args);
    }
    jfloat CallNonvirtualFloatMethodA(jobject obj, jclass clazz, 
				      jmethodID methodID, 
				      jvalue * args) {
        return functions->CallNonvirtualFloatMethodA(this,obj,clazz,
						     methodID,args);
    }

    jdouble CallNonvirtualDoubleMethod(jobject obj, jclass clazz,
				       jmethodID methodID, ...) {
        va_list args;
	jdouble result;
	va_start(args,methodID);
	result = functions->CallNonvirtualDoubleMethodV(this,obj,clazz,
							methodID,args);
	va_end(args);
	return result;
    }
    jdouble CallNonvirtualDoubleMethodV(jobject obj, jclass clazz,
					jmethodID methodID, 
					va_list args) {
        return functions->CallNonvirtualDoubleMethodV(this,obj,clazz,
						      methodID,args);
    }
    jdouble CallNonvirtualDoubleMethodA(jobject obj, jclass clazz, 
					jmethodID methodID, 
					jvalue * args) {
        return functions->CallNonvirtualDoubleMethodA(this,obj,clazz,
						      methodID,args);
    }

    void CallNonvirtualVoidMethod(jobject obj, jclass clazz,
				  jmethodID methodID, ...) {
        va_list args;
	va_start(args,methodID);
	functions->CallNonvirtualVoidMethodV(this,obj,clazz,methodID,args);
	va_end(args);
    }
    void CallNonvirtualVoidMethodV(jobject obj, jclass clazz,
				   jmethodID methodID, 
				   va_list args) {
        functions->CallNonvirtualVoidMethodV(this,obj,clazz,methodID,args);
    }
    void CallNonvirtualVoidMethodA(jobject obj, jclass clazz,
				   jmethodID methodID, 
				   jvalue * args) {
        functions->CallNonvirtualVoidMethodA(this,obj,clazz,methodID,args);
    }

    jfieldID GetFieldID(jclass clazz, const char *name, 
			const char *sig) {
        return functions->GetFieldID(this,clazz,name,sig);
    }

    jobject GetObjectField(jobject obj, jfieldID fieldID) {
        return functions->GetObjectField(this,obj,fieldID);
    }
    jboolean GetBooleanField(jobject obj, jfieldID fieldID) {
        return functions->GetBooleanField(this,obj,fieldID);
    }
    jbyte GetByteField(jobject obj, jfieldID fieldID) {
        return functions->GetByteField(this,obj,fieldID);
    }
    jchar GetCharField(jobject obj, jfieldID fieldID) {
        return functions->GetCharField(this,obj,fieldID);
    }
    jshort GetShortField(jobject obj, jfieldID fieldID) {
        return functions->GetShortField(this,obj,fieldID);
    }
    jint GetIntField(jobject obj, jfieldID fieldID) {
        return functions->GetIntField(this,obj,fieldID);
    }
    jlong GetLongField(jobject obj, jfieldID fieldID) {
        return functions->GetLongField(this,obj,fieldID);
    }
    jfloat GetFloatField(jobject obj, jfieldID fieldID) {
        return functions->GetFloatField(this,obj,fieldID);
    }
    jdouble GetDoubleField(jobject obj, jfieldID fieldID) {
        return functions->GetDoubleField(this,obj,fieldID);
    }

    void SetObjectField(jobject obj, jfieldID fieldID, jobject val) {
        functions->SetObjectField(this,obj,fieldID,val);
    }
    void SetBooleanField(jobject obj, jfieldID fieldID, 
			 jboolean val) {
        functions->SetBooleanField(this,obj,fieldID,val);
    }
    void SetByteField(jobject obj, jfieldID fieldID, 
		      jbyte val) {
        functions->SetByteField(this,obj,fieldID,val);
    }
    void SetCharField(jobject obj, jfieldID fieldID, 
		      jchar val) {
        functions->SetCharField(this,obj,fieldID,val);
    }
    void SetShortField(jobject obj, jfieldID fieldID,
		       jshort val) {
        functions->SetShortField(this,obj,fieldID,val);
    }
    void SetIntField(jobject obj, jfieldID fieldID, 
		     jint val) {
        functions->SetIntField(this,obj,fieldID,val);
    }
    void SetLongField(jobject obj, jfieldID fieldID, 
		      jlong val) {
        functions->SetLongField(this,obj,fieldID,val);
    }
    void SetFloatField(jobject obj, jfieldID fieldID, 
		       jfloat val) {
        functions->SetFloatField(this,obj,fieldID,val);
    }
    void SetDoubleField(jobject obj, jfieldID fieldID, 
			jdouble val) {
        functions->SetDoubleField(this,obj,fieldID,val);
    }

    jmethodID GetStaticMethodID(jclass clazz, const char *name, 
				const char *sig) {
        return functions->GetStaticMethodID(this,clazz,name,sig);
    }

    jobject CallStaticObjectMethod(jclass clazz, jmethodID methodID, 
			     ...) {
        va_list args;
	jobject result;
	va_start(args,methodID);
	result = functions->CallStaticObjectMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jobject CallStaticObjectMethodV(jclass clazz, jmethodID methodID, 
			      va_list args) {
        return functions->CallStaticObjectMethodV(this,clazz,methodID,args);
    }
    jobject CallStaticObjectMethodA(jclass clazz, jmethodID methodID, 
			      jvalue *args) {
        return functions->CallStaticObjectMethodA(this,clazz,methodID,args);
    }

    jboolean CallStaticBooleanMethod(jclass clazz, 
				     jmethodID methodID, ...) {
        va_list args;
	jboolean result;
	va_start(args,methodID);
	result = functions->CallStaticBooleanMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jboolean CallStaticBooleanMethodV(jclass clazz,
				      jmethodID methodID, va_list args) {
        return functions->CallStaticBooleanMethodV(this,clazz,methodID,args);
    }
    jboolean CallStaticBooleanMethodA(jclass clazz,
				      jmethodID methodID, jvalue *args) {
        return functions->CallStaticBooleanMethodA(this,clazz,methodID,args);
    }

    jbyte CallStaticByteMethod(jclass clazz,
			       jmethodID methodID, ...) {
        va_list args;
	jbyte result;
	va_start(args,methodID);
	result = functions->CallStaticByteMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jbyte CallStaticByteMethodV(jclass clazz,
				jmethodID methodID, va_list args) {
        return functions->CallStaticByteMethodV(this,clazz,methodID,args);
    }
    jbyte CallStaticByteMethodA(jclass clazz, 
				jmethodID methodID, jvalue *args) {
        return functions->CallStaticByteMethodA(this,clazz,methodID,args);
    }

    jchar CallStaticCharMethod(jclass clazz,
			       jmethodID methodID, ...) {
        va_list args;
	jchar result;
	va_start(args,methodID);
	result = functions->CallStaticCharMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jchar CallStaticCharMethodV(jclass clazz,
				jmethodID methodID, va_list args) {
        return functions->CallStaticCharMethodV(this,clazz,methodID,args);
    }
    jchar CallStaticCharMethodA(jclass clazz,
				jmethodID methodID, jvalue *args) {
        return functions->CallStaticCharMethodA(this,clazz,methodID,args);
    }

    jshort CallStaticShortMethod(jclass clazz,
				 jmethodID methodID, ...) {
        va_list args;
	jshort result;
	va_start(args,methodID);
	result = functions->CallStaticShortMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jshort CallStaticShortMethodV(jclass clazz,
				  jmethodID methodID, va_list args) {
        return functions->CallStaticShortMethodV(this,clazz,methodID,args);
    }
    jshort CallStaticShortMethodA(jclass clazz,
				  jmethodID methodID, jvalue *args) {
        return functions->CallStaticShortMethodA(this,clazz,methodID,args);
    }

    jint CallStaticIntMethod(jclass clazz,
			     jmethodID methodID, ...) {
        va_list args;
	jint result;
	va_start(args,methodID);
	result = functions->CallStaticIntMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jint CallStaticIntMethodV(jclass clazz,
			      jmethodID methodID, va_list args) {
        return functions->CallStaticIntMethodV(this,clazz,methodID,args);
    }
    jint CallStaticIntMethodA(jclass clazz, 
			      jmethodID methodID, jvalue *args) {
        return functions->CallStaticIntMethodA(this,clazz,methodID,args);
    }

    jlong CallStaticLongMethod(jclass clazz,
			       jmethodID methodID, ...) {
        va_list args;
	jlong result;
	va_start(args,methodID);
	result = functions->CallStaticLongMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jlong CallStaticLongMethodV(jclass clazz, 
				jmethodID methodID, va_list args) {
        return functions->CallStaticLongMethodV(this,clazz,methodID,args);
    }
    jlong CallStaticLongMethodA(jclass clazz, 
				jmethodID methodID, jvalue *args) {
        return functions->CallStaticLongMethodA(this,clazz,methodID,args);
    }

    jfloat CallStaticFloatMethod(jclass clazz, 
				 jmethodID methodID, ...) {
        va_list args;
	jfloat result;
	va_start(args,methodID);
	result = functions->CallStaticFloatMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jfloat CallStaticFloatMethodV(jclass clazz, 
				  jmethodID methodID, va_list args) {
        return functions->CallStaticFloatMethodV(this,clazz,methodID,args);
    }
    jfloat CallStaticFloatMethodA(jclass clazz, 
				  jmethodID methodID, jvalue *args) {
        return functions->CallStaticFloatMethodA(this,clazz,methodID,args);
    }

    jdouble CallStaticDoubleMethod(jclass clazz, 
				   jmethodID methodID, ...) {
        va_list args;
	jdouble result;
	va_start(args,methodID);
	result = functions->CallStaticDoubleMethodV(this,clazz,methodID,args);
	va_end(args);
	return result;
    }
    jdouble CallStaticDoubleMethodV(jclass clazz, 
				    jmethodID methodID, va_list args) {
        return functions->CallStaticDoubleMethodV(this,clazz,methodID,args);
    }
    jdouble CallStaticDoubleMethodA(jclass clazz, 
				    jmethodID methodID, jvalue *args) {
        return functions->CallStaticDoubleMethodA(this,clazz,methodID,args);
    }

    void CallStaticVoidMethod(jclass cls, jmethodID methodID, ...) {
        va_list args;
	va_start(args,methodID);
	functions->CallStaticVoidMethodV(this,cls,methodID,args);
	va_end(args);
    }
    void CallStaticVoidMethodV(jclass cls, jmethodID methodID, 
			       va_list args) {
        functions->CallStaticVoidMethodV(this,cls,methodID,args);
    }
    void CallStaticVoidMethodA(jclass cls, jmethodID methodID, 
			       jvalue * args) {
        functions->CallStaticVoidMethodA(this,cls,methodID,args);
    }

    jfieldID GetStaticFieldID(jclass clazz, const char *name, 
			      const char *sig) {
        return functions->GetStaticFieldID(this,clazz,name,sig);
    }
    jobject GetStaticObjectField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticObjectField(this,clazz,fieldID);
    }
    jboolean GetStaticBooleanField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticBooleanField(this,clazz,fieldID);
    }
    jbyte GetStaticByteField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticByteField(this,clazz,fieldID);
    }
    jchar GetStaticCharField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticCharField(this,clazz,fieldID);
    }
    jshort GetStaticShortField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticShortField(this,clazz,fieldID);
    }
    jint GetStaticIntField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticIntField(this,clazz,fieldID);
    }
    jlong GetStaticLongField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticLongField(this,clazz,fieldID);
    }
    jfloat GetStaticFloatField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticFloatField(this,clazz,fieldID);
    }
    jdouble GetStaticDoubleField(jclass clazz, jfieldID fieldID) {
        return functions->GetStaticDoubleField(this,clazz,fieldID);
    }

    void SetStaticObjectField(jclass clazz, jfieldID fieldID,
			jobject value) {
      functions->SetStaticObjectField(this,clazz,fieldID,value);
    }
    void SetStaticBooleanField(jclass clazz, jfieldID fieldID,
			jboolean value) {
      functions->SetStaticBooleanField(this,clazz,fieldID,value);
    }
    void SetStaticByteField(jclass clazz, jfieldID fieldID,
			jbyte value) {
      functions->SetStaticByteField(this,clazz,fieldID,value);
    }
    void SetStaticCharField(jclass clazz, jfieldID fieldID,
			jchar value) {
      functions->SetStaticCharField(this,clazz,fieldID,value);
    }
    void SetStaticShortField(jclass clazz, jfieldID fieldID,
			jshort value) {
      functions->SetStaticShortField(this,clazz,fieldID,value);
    }
    void SetStaticIntField(jclass clazz, jfieldID fieldID,
			jint value) {
      functions->SetStaticIntField(this,clazz,fieldID,value);
    }
    void SetStaticLongField(jclass clazz, jfieldID fieldID,
			jlong value) {
      functions->SetStaticLongField(this,clazz,fieldID,value);
    }
    void SetStaticFloatField(jclass clazz, jfieldID fieldID,
			jfloat value) {
      functions->SetStaticFloatField(this,clazz,fieldID,value);
    }
    void SetStaticDoubleField(jclass clazz, jfieldID fieldID,
			jdouble value) {
      functions->SetStaticDoubleField(this,clazz,fieldID,value);
    }

    jstring NewString(const jchar *unicode, jsize len) {
        return functions->NewString(this,unicode,len);
    }
    jsize GetStringLength(jstring str) {
        return functions->GetStringLength(this,str);
    }
    const jchar *GetStringChars(jstring str, jboolean *isCopy) {
        return functions->GetStringChars(this,str,isCopy);
    }
    void ReleaseStringChars(jstring str, const jchar *chars) {
        functions->ReleaseStringChars(this,str,chars);
    }
  
    jstring NewStringUTF(const char *utf) {
        return functions->NewStringUTF(this,utf);
    }
    jsize GetStringUTFLength(jstring str) {
        return functions->GetStringUTFLength(this,str);
    }
    const char* GetStringUTFChars(jstring str, jboolean *isCopy) {
        return functions->GetStringUTFChars(this,str,isCopy);
    }
    void ReleaseStringUTFChars(jstring str, const char* chars) {
        functions->ReleaseStringUTFChars(this,str,chars);
    }

    jsize GetArrayLength(jarray array) {
        return functions->GetArrayLength(this,array);
    }

    jobjectArray NewObjectArray(jsize len, jclass clazz, 
				jobject init) {
        return functions->NewObjectArray(this,len,clazz,init);
    }
    jobject GetObjectArrayElement(jobjectArray array, jsize index) {
        return functions->GetObjectArrayElement(this,array,index);
    }
    void SetObjectArrayElement(jobjectArray array, jsize index, 
			       jobject val) {
        functions->SetObjectArrayElement(this,array,index,val);
    }

    jbooleanArray NewBooleanArray(jsize len) {
        return functions->NewBooleanArray(this,len);
    }
    jbyteArray NewByteArray(jsize len) {
        return functions->NewByteArray(this,len);
    }
    jcharArray NewCharArray(jsize len) {
        return functions->NewCharArray(this,len);
    }
    jshortArray NewShortArray(jsize len) {
        return functions->NewShortArray(this,len);
    }
    jintArray NewIntArray(jsize len) {
        return functions->NewIntArray(this,len);
    }
    jlongArray NewLongArray(jsize len) {
        return functions->NewLongArray(this,len);
    }
    jfloatArray NewFloatArray(jsize len) {
        return functions->NewFloatArray(this,len);
    }
    jdoubleArray NewDoubleArray(jsize len) {
        return functions->NewDoubleArray(this,len);
    }

    jboolean * GetBooleanArrayElements(jbooleanArray array, jboolean *isCopy) {
        return functions->GetBooleanArrayElements(this,array,isCopy);
    }
    jbyte * GetByteArrayElements(jbyteArray array, jboolean *isCopy) {
        return functions->GetByteArrayElements(this,array,isCopy);
    }
    jchar * GetCharArrayElements(jcharArray array, jboolean *isCopy) {
        return functions->GetCharArrayElements(this,array,isCopy);
    }
    jshort * GetShortArrayElements(jshortArray array, jboolean *isCopy) {
        return functions->GetShortArrayElements(this,array,isCopy);
    }
    jint * GetIntArrayElements(jintArray array, jboolean *isCopy) {
        return functions->GetIntArrayElements(this,array,isCopy);
    }
    jlong * GetLongArrayElements(jlongArray array, jboolean *isCopy) {
        return functions->GetLongArrayElements(this,array,isCopy);
    }
    jfloat * GetFloatArrayElements(jfloatArray array, jboolean *isCopy) {
        return functions->GetFloatArrayElements(this,array,isCopy);
    }
    jdouble * GetDoubleArrayElements(jdoubleArray array, jboolean *isCopy) {
        return functions->GetDoubleArrayElements(this,array,isCopy);
    }

    void ReleaseBooleanArrayElements(jbooleanArray array, 
				     jboolean *elems,
				     jint mode) {
        functions->ReleaseBooleanArrayElements(this,array,elems,mode);
    }
    void ReleaseByteArrayElements(jbyteArray array, 
				  jbyte *elems,
				  jint mode) {
        functions->ReleaseByteArrayElements(this,array,elems,mode);
    }
    void ReleaseCharArrayElements(jcharArray array, 
				  jchar *elems,
				  jint mode) {
        functions->ReleaseCharArrayElements(this,array,elems,mode);
    }
    void ReleaseShortArrayElements(jshortArray array, 
				   jshort *elems,
				   jint mode) {
        functions->ReleaseShortArrayElements(this,array,elems,mode);
    }
    void ReleaseIntArrayElements(jintArray array, 
				 jint *elems,
				 jint mode) {
        functions->ReleaseIntArrayElements(this,array,elems,mode);
    }
    void ReleaseLongArrayElements(jlongArray array, 
				  jlong *elems,
				  jint mode) {
        functions->ReleaseLongArrayElements(this,array,elems,mode);
    }
    void ReleaseFloatArrayElements(jfloatArray array, 
				   jfloat *elems,
				   jint mode) {
        functions->ReleaseFloatArrayElements(this,array,elems,mode);
    }
    void ReleaseDoubleArrayElements(jdoubleArray array, 
				    jdouble *elems,
				    jint mode) {
        functions->ReleaseDoubleArrayElements(this,array,elems,mode);
    }

    void GetBooleanArrayRegion(jbooleanArray array, 
			       jsize start, jsize len, jboolean *buf) {
        functions->GetBooleanArrayRegion(this,array,start,len,buf);
    }
    void GetByteArrayRegion(jbyteArray array, 
			    jsize start, jsize len, jbyte *buf) {
        functions->GetByteArrayRegion(this,array,start,len,buf);
    }
    void GetCharArrayRegion(jcharArray array, 
			    jsize start, jsize len, jchar *buf) {
        functions->GetCharArrayRegion(this,array,start,len,buf);
    }
    void GetShortArrayRegion(jshortArray array, 
			     jsize start, jsize len, jshort *buf) {
        functions->GetShortArrayRegion(this,array,start,len,buf);
    }
    void GetIntArrayRegion(jintArray array, 
			   jsize start, jsize len, jint *buf) {
        functions->GetIntArrayRegion(this,array,start,len,buf);
    }
    void GetLongArrayRegion(jlongArray array, 
			    jsize start, jsize len, jlong *buf) {
        functions->GetLongArrayRegion(this,array,start,len,buf);
    }
    void GetFloatArrayRegion(jfloatArray array, 
			     jsize start, jsize len, jfloat *buf) {
        functions->GetFloatArrayRegion(this,array,start,len,buf);
    }
    void GetDoubleArrayRegion(jdoubleArray array, 
			      jsize start, jsize len, jdouble *buf) {
        functions->GetDoubleArrayRegion(this,array,start,len,buf);
    }

    void SetBooleanArrayRegion(jbooleanArray array, jsize start, jsize len, 
			       jboolean *buf) {
        functions->SetBooleanArrayRegion(this,array,start,len,buf);
    }
    void SetByteArrayRegion(jbyteArray array, jsize start, jsize len,
			    jbyte *buf) {
        functions->SetByteArrayRegion(this,array,start,len,buf);
    }
    void SetCharArrayRegion(jcharArray array, jsize start, jsize len, 
			    jchar *buf) {
        functions->SetCharArrayRegion(this,array,start,len,buf);
    }
    void SetShortArrayRegion(jshortArray array, jsize start, jsize len, 
			     jshort *buf) {
        functions->SetShortArrayRegion(this,array,start,len,buf);
    }
    void SetIntArrayRegion(jintArray array, jsize start, jsize len,
			   jint *buf) {
        functions->SetIntArrayRegion(this,array,start,len,buf);
    }
    void SetLongArrayRegion(jlongArray array, jsize start, jsize len,
			    jlong *buf) {
        functions->SetLongArrayRegion(this,array,start,len,buf);
    }
    void SetFloatArrayRegion(jfloatArray array, jsize start, jsize len, 
			     jfloat *buf) {
        functions->SetFloatArrayRegion(this,array,start,len,buf);
    }
    void SetDoubleArrayRegion(jdoubleArray array, jsize start, jsize len,
			      jdouble *buf) {
        functions->SetDoubleArrayRegion(this,array,start,len,buf);
    }

    jint RegisterNatives(jclass clazz, const RemoteJNINativeMethod *methods,
			 jint nMethods) {
        return functions->RegisterNatives(this,clazz,methods,nMethods);
    }
    jint UnregisterNatives(jclass clazz) {
        return functions->UnregisterNatives(this,clazz);
    }  
   
    jint MonitorEnter(jobject obj) {
        return functions->MonitorEnter(this,obj);
    }
    jint MonitorExit(jobject obj) {
        return functions->MonitorExit(this,obj);
    }

    jint GetJavaVM(void **vm) {
        return functions->GetJavaVM(this,vm);
    }

    void GetStringRegion(jstring str, jsize start, jsize len, jchar *buf) {
        functions->GetStringRegion(this,str,start,len,buf);
    }
    void GetStringUTFRegion(jstring str, jsize start, jsize len, char *buf) {
        functions->GetStringUTFRegion(this,str,start,len,buf);
    }

    void * GetPrimitiveArrayCritical(jarray array, jboolean *isCopy) {
        return functions->GetPrimitiveArrayCritical(this,array,isCopy);
    }
    void ReleasePrimitiveArrayCritical(jarray array, void *carray, jint mode) {
        functions->ReleasePrimitiveArrayCritical(this,array,carray,mode);
    }

    const jchar * GetStringCritical(jstring string, jboolean *isCopy) {
        return functions->GetStringCritical(this,string,isCopy);
    }
    void ReleaseStringCritical(jstring string, const jchar *cstring) {
        functions->ReleaseStringCritical(this,string,cstring);
    }

    jweak NewWeakGlobalRef(jobject obj) {
        return functions->NewWeakGlobalRef(this,obj);
    }
    void DeleteWeakGlobalRef(jweak ref) {
        functions->DeleteWeakGlobalRef(this,ref);
    }

    jboolean ExceptionCheck() {
	return functions->ExceptionCheck(this);
    }

    /*XXXBegin New functions, not in a standard JNIEnv */
    int SecureNewObject(jclass clazz, jmethodID methodID, jvalue* args,
			 jobject* result, ISecurityContext* ctx) {
	return functions->SecureNewObject(this, clazz, 
					  methodID, args, result, ctx);
    }

    int SecureCallMethod(jd_jni_type type, jobject obj, jmethodID methodID,
			  jvalue* args, jvalue* result,  
			  ISecurityContext* ctx) {
	return functions->SecureCallMethod(this, type, obj, methodID,
			  args, result,  ctx);
    }

    int SecureCallNonvirtualMethod(jd_jni_type type, jobject obj, jclass clazz,
				    jmethodID methodID, jvalue* args,
				    jvalue* result, ISecurityContext* ctx) {
	return functions->SecureCallNonvirtualMethod(this, type, obj, clazz,
				    methodID, args,
				    result, ctx);
    }

    int SecureGetField(jd_jni_type type, jobject obj, jfieldID fieldID, 
			jvalue* result, ISecurityContext* ctx) {
	return functions->SecureGetField(this, type, obj, fieldID, result, ctx);
    }

    int SecureSetField(jd_jni_type type, jobject obj, jfieldID fieldID, 
			jvalue result, ISecurityContext* ctx) {
	return functions->SecureSetField(this, type, obj, fieldID, result, ctx);
    }


    int SecureCallStaticMethod(jd_jni_type type,
				    jclass clazz,
				    jmethodID methodID,
				    jvalue* args,
				    jvalue* result,
				    ISecurityContext* ctx) {
	return functions->SecureCallStaticMethod(this, type, clazz,
				    methodID, args, result, ctx);
    }

    int SecureGetStaticField(jd_jni_type type,
			      jclass clazz,
			      jfieldID fieldID,
			      jvalue* result,
			      ISecurityContext* ctx) {
	return functions->SecureGetStaticField(this, type, clazz, fieldID, 
					result, ctx);

    }

    int SecureSetStaticField(jd_jni_type type,
			      jclass clazz,
			      jfieldID fieldID,
			      jvalue val,
			      ISecurityContext* ctx) {
	return functions->SecureSetStaticField(this, type, clazz, fieldID, val, ctx);

    }

    int AddRef() {
	functions->environmentInfo->refcnt++;
	return GetRefCnt();
    }

    int Release() {
	functions->environmentInfo->refcnt--;
	return GetRefCnt();
    }

    int GetRefCnt() {
	return functions->environmentInfo->refcnt;
    }

    JavaPluginFactory5* GetFactory() {
	return functions->environmentInfo->factory;
    }

    void* GetPipe() {
	return functions->environmentInfo->pipe;
    }

    /* check whether an action is ok on a target in javascript based
       on the check in java */
    int CSecurityContextImplies(jobject context, 
				const char* target, 
				const char* action) {
	return functions->CSecurityContextImplies(this, context, 
						  target, action);
    }

    /*XXXEnd */

#endif /* __cplusplus */
};

RemoteJNIEnv* create_RemoteJNIEnv();

void init_RemoteJNIEnv(RemoteJNIEnv* env, int env_index, void* pipe);

  /* Delete the remote JNI env */
void dispose_RemoteJNIEnv(RemoteJNIEnv* env);

void send_msg(RemoteJNIEnv* env, void *buffer, int length);

int get_msg(RemoteJNIEnv *env, void *buffer, int length);

#define REMOTEJNI_VERSION_1_1 0x00010001
#define REMOTEJNI_VERSION_1_2 0x00010002

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* REMOTEJNI_H */


