/*
 * @(#)ISecureEnv.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// ISecureEnv.h  by X.Lu
//
///=--------------------------------------------------------------------------=
// Contains additional interface for Plugin Tag information called by Plugin.
//
#ifndef _ISECUREENV_H_
#define _ISECUREENV_H_

#include "ISupports.h"
#include "IFactory.h"
#include "ISecurityContext.h"

#include "jni.h"
//{389E0AC0-9840-11d6-9A73-00B0D0A18D51}
#define ISECUREENV_IID \
    {0x389E0AC0, 0x9840, 0x11d6, {0x9A, 0x73, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}}

enum jd_jni_type
{
    jd_jobject_type = 0,
    jd_jboolean_type,
    jd_jbyte_type,
    jd_jchar_type,
    jd_jshort_type,
    jd_jint_type,
    jd_jlong_type,
    jd_jfloat_type,
    jd_jdouble_type,
    jd_jvoid_type
};

//ISupports interface (A replicate of nsISupports)
class ISecureEnv : public ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(ISECUREENV_IID);
    /**
     * Create new Java object in LiveConnect.
     *
     * @param clazz      -- Java Class object.
     * @param methodID   -- Method id
     * @param args       -- arguments for invoking the constructor.
     * @param result     -- return new Java object.
     * @param ctx        -- security context
     */
    JD_IMETHOD NewObject(/*[in]*/ jclass clazz,
                         /*[in]*/  jmethodID methodID,
                         /*[in]*/  jvalue *args,
                         /*[out]*/ jobject* result,
                         /*[in]*/  ISecurityContext* ctx = NULL) = 0;

    /**
     * Invoke method on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param obj        -- Java object.
     * @param methodID   -- method id
     * @param args       -- arguments for invoking the constructor.
     * @param result     -- return result of invocation.
     * @param ctx        -- security context
     */
    JD_IMETHOD CallMethod(/*[in]*/ jd_jni_type type,
                          /*[in]*/  jobject obj,
                          /*[in]*/  jmethodID methodID,
                          /*[in]*/  jvalue *args,
                          /*[out]*/ jvalue* result,
                          /*[in]*/  ISecurityContext* ctx = NULL) = 0;

    /**
     * Invoke non-virtual method on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param obj        -- Java object.
     * @param clazz      -- Class object
     * @param methodID   -- method id
     * @param args       -- arguments for invoking the constructor.
     * @param ctx        -- security context
     * @param result     -- return result of invocation.
     */
    JD_IMETHOD CallNonvirtualMethod(/*[in]*/ jd_jni_type type,
                                    /*[in]*/  jobject obj,
                                    /*[in]*/  jclass clazz,
                                    /*[in]*/  jmethodID methodID,
                                    /*[in]*/  jvalue *args,
                                    /*[out]*/ jvalue* result,
                                    /*[in]*/  ISecurityContext* ctx = NULL) = 0;

    /**
     * Get a field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param obj        -- Java object.
     * @param fieldID    -- field id
     * @param result     -- return field value
     * @param ctx        -- security context
     */
    JD_IMETHOD GetField(/*[in]*/ jd_jni_type type,
                        /*[in]*/  jobject obj,
                        /*[in]*/  jfieldID fieldID,
                        /*[out]*/ jvalue* result,
                        /*[in]*/  ISecurityContext* ctx = NULL) = 0;

    /**
     * Set a field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param obj        -- Java object.
     * @param fieldID    -- field id
     * @param val        -- field value to set
     * @param ctx        -- security context
     */
    JD_IMETHOD SetField(/*[in]*/ jd_jni_type type,
                        /*[in]*/ jobject obj,
                        /*[in]*/ jfieldID fieldID,
                        /*[in]*/ jvalue val,
                        /*[in]*/ ISecurityContext* ctx = NULL) = 0;

    /**
     * Invoke static method on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param clazz      -- Class object.
     * @param methodID   -- method id
     * @param args       -- arguments for invoking the constructor.
     * @param result     -- return result of invocation.
     * @param ctx        -- security context
     */
    JD_IMETHOD CallStaticMethod(/*[in]*/  jd_jni_type type,
                                /*[in]*/  jclass clazz,
                                /*[in]*/  jmethodID methodID,
                                /*[in]*/  jvalue *args,
                                /*[out]*/ jvalue* result,
                                /*[in]*/  ISecurityContext* ctx = NULL) = 0;

    /**
     * Get a static field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param clazz      -- Class object.
     * @param fieldID    -- field id
     * @param result     -- return field value
     * @param ctx        -- security context
     */
    JD_IMETHOD GetStaticField(/*[in]*/  jd_jni_type type,
                              /*[in]*/  jclass clazz,
                              /*[in]*/  jfieldID fieldID,
                              /*[out]*/ jvalue* result,
                              /*[in]*/  ISecurityContext* ctx = NULL) = 0;


    /**
     * Set a static field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param clazz      -- Class object.
     * @param fieldID    -- field id
     * @param val        -- field value to set
     * @param ctx        -- security context
     */
    JD_IMETHOD SetStaticField(/*[in]*/ jd_jni_type type,
                              /*[in]*/ jclass clazz,
                              /*[in]*/ jfieldID fieldID,
                              /*[in]*/ jvalue val,
                              /*[in]*/ ISecurityContext* ctx = NULL) = 0;


    JD_IMETHOD GetVersion(/*[out]*/ jint* version) = 0;

    JD_IMETHOD DefineClass(/*[in]*/  const char* name,
                           /*[in]*/  jobject loader,
                           /*[in]*/  const jbyte *buf,
                           /*[in]*/  jsize len,
                           /*[out]*/ jclass* clazz) = 0;

    JD_IMETHOD FindClass(/*[in]*/  const char* name,
                         /*[out]*/ jclass* clazz) = 0;

    JD_IMETHOD GetSuperclass(/*[in]*/  jclass sub,
                             /*[out]*/ jclass* super) = 0;

    JD_IMETHOD IsAssignableFrom(/*[in]*/  jclass sub,
                                /*[in]*/  jclass super,
                                /*[out]*/ jboolean* result) = 0;

    JD_IMETHOD Throw(/*[in]*/  jthrowable obj,
                     /*[out]*/ jint* result) = 0;

    JD_IMETHOD ThrowNew(/*[in]*/  jclass clazz,
                        /*[in]*/  const char *msg,
                        /*[out]*/ jint* result) = 0;

    JD_IMETHOD ExceptionOccurred(/*[out]*/ jthrowable* result) = 0;

    JD_IMETHOD ExceptionDescribe(void) = 0;

    JD_IMETHOD ExceptionClear(void) = 0;

    JD_IMETHOD FatalError(/*[in]*/ const char* msg) = 0;

    JD_IMETHOD NewGlobalRef(/*[in]*/  jobject lobj,
                            /*[out]*/ jobject* result) = 0;

    JD_IMETHOD DeleteGlobalRef(/*[in]*/ jobject gref) = 0;

    JD_IMETHOD DeleteLocalRef(/*[in]*/ jobject obj) = 0;

    JD_IMETHOD IsSameObject(/*[in]*/  jobject obj1,
                            /*[in]*/  jobject obj2,
                            /*[out]*/ jboolean* result) = 0;

    JD_IMETHOD AllocObject(/*[in]*/  jclass clazz,
                           /*[out]*/ jobject* result) = 0;

    JD_IMETHOD GetObjectClass(/*[in]*/  jobject obj,
                              /*[out]*/ jclass* result) = 0;

    JD_IMETHOD IsInstanceOf(/*[in]*/  jobject obj,
                            /*[in]*/  jclass clazz,
                            /*[out]*/ jboolean* result) = 0;

    JD_IMETHOD GetMethodID(/*[in]*/  jclass clazz,
                           /*[in]*/  const char* name,
                           /*[in]*/  const char* sig,
                           /*[out]*/ jmethodID* id) = 0;

    JD_IMETHOD GetFieldID(/*[in]*/  jclass clazz,
                          /*[in]*/  const char* name,
                          /*[in]*/  const char* sig,
                          /*[out]*/ jfieldID* id) = 0;

    JD_IMETHOD GetStaticMethodID(/*[in]*/  jclass clazz,
                                 /*[in]*/  const char* name,
                                 /*[in]*/  const char* sig,
                                 /*[out]*/ jmethodID* id) = 0;

    JD_IMETHOD GetStaticFieldID(/*[in]*/  jclass clazz,
                                /*[in]*/  const char* name,
                                /*[in]*/  const char* sig,
                                /*[out]*/ jfieldID* id) = 0;

    JD_IMETHOD NewString(/*[in]*/  const jchar* unicode,
                         /*[in]*/  jsize len,
                         /*[out]*/ jstring* result) = 0;

    JD_IMETHOD GetStringLength(/*[in]*/  jstring str,
                               /*[out]*/ jsize* result) = 0;

    JD_IMETHOD GetStringChars(/*[in]*/  jstring str,
                              /*[in]*/  jboolean *isCopy,
                              /*[out]*/ const jchar** result) = 0;

    JD_IMETHOD ReleaseStringChars(/*[in]*/  jstring str,
                                  /*[in]*/  const jchar *chars) = 0;

    JD_IMETHOD NewStringUTF(/*[in]*/  const char *utf,
                            /*[out]*/ jstring* result) = 0;

    JD_IMETHOD GetStringUTFLength(/*[in]*/  jstring str,
                                  /*[out]*/ jsize* result) = 0;

    JD_IMETHOD GetStringUTFChars(/*[in]*/  jstring str,
                                 /*[in]*/  jboolean *isCopy,
                                 /*[out]*/ const char** result) = 0;

    JD_IMETHOD ReleaseStringUTFChars(/*[in]*/  jstring str,
                                     /*[in]*/  const char *chars) = 0;

    JD_IMETHOD GetArrayLength(/*[in]*/  jarray array,
                              /*[out]*/ jsize* result) = 0;

    JD_IMETHOD NewObjectArray(/*[in]*/  jsize len,
    					/*[in]*/  jclass clazz,
                        /*[in]*/  jobject init,
                        /*[out]*/ jobjectArray* result) = 0;

    JD_IMETHOD GetObjectArrayElement(/*[in]*/  jobjectArray array,
                                     /*[in]*/  jsize index,
                                     /*[out]*/ jobject* result) = 0;

    JD_IMETHOD SetObjectArrayElement(/*[in]*/  jobjectArray array,
                                     /*[in]*/  jsize index,
                                     /*[in]*/  jobject val) = 0;

    JD_IMETHOD NewArray(/*[in]*/ jd_jni_type element_type,
                        /*[in]*/  jsize len,
                        /*[out]*/ jarray* result) = 0;

    JD_IMETHOD GetArrayElements(/*[in]*/  jd_jni_type type,
                                /*[in]*/  jarray array,
                                /*[in]*/  jboolean *isCopy,
                                /*[out]*/ void* result) = 0;

    JD_IMETHOD ReleaseArrayElements(/*[in]*/ jd_jni_type type,
                                    /*[in]*/ jarray array,
                                    /*[in]*/ void *elems,
                                    /*[in]*/ jint mode) = 0;

    JD_IMETHOD GetArrayRegion(/*[in]*/  jd_jni_type type,
                              /*[in]*/  jarray array,
                              /*[in]*/  jsize start,
                              /*[in]*/  jsize len,
                              /*[out]*/ void* buf) = 0;

    JD_IMETHOD SetArrayRegion(/*[in]*/  jd_jni_type type,
                              /*[in]*/  jarray array,
                              /*[in]*/  jsize start,
                              /*[in]*/  jsize len,
                              /*[in]*/  void* buf) = 0;

    JD_IMETHOD RegisterNatives(/*[in]*/  jclass clazz,
                               /*[in]*/  const JNINativeMethod *methods,
                               /*[in]*/  jint nMethods,
                               /*[out]*/ jint* result) = 0;

    JD_IMETHOD UnregisterNatives(/*[in]*/  jclass clazz,
                                 /*[out]*/ jint* result) = 0;

    JD_IMETHOD MonitorEnter(/*[in]*/  jobject obj,
                            /*[out]*/ jint* result) = 0;

    JD_IMETHOD MonitorExit(/*[in]*/  jobject obj,
                           /*[out]*/ jint* result) = 0;

    JD_IMETHOD GetJavaVM(/*[in]*/  JavaVM **vm,
                         /*[out]*/ jint* result) = 0;
};

#endif //_ISECUREENV_H
