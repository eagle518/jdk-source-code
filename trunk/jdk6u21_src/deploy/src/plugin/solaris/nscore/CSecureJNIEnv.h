/*
 * @(#)CSecureJNIEnv.h	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CSecureJNIEnv.h  by Stanley Man-Kit Ho
//
///=--------------------------------------------------------------------------=

#ifndef CSecureJNIEnv_h___
#define CSecureJNIEnv_h___

#include "JDSupportUtils.h"
#include "ISecureEnv.h"
#include "remotejni.h"
#include "jni.h"

class ISecurityContext;
class CSecureJNIEnv : public ISecureEnv
{
public:

    ////////////////////////////////////////////////////////////////////////////
    // from ISupports and AggregatedQueryInterface:

    JD_DECL_AGGREGATED

    static JD_METHOD Create(ISupports* outer, RemoteJNIEnv* env, const JDIID& aIID, void* *aInstancePtr);

    ////////////////////////////////////////////////////////////////////////////
    // from ISecureJNIEnv:


    /**
     * Create new Java object in LiveConnect.
     *
     * @param clazz      -- Java Class object.
     * @param methodID   -- Method id
     * @param args       -- arguments for invoking the constructor.
     * @param result     -- return new Java object.
     * @param ctx        -- security context 
     */
    JD_IMETHOD NewObject(/*[in]*/  jclass clazz, 
                         /*[in]*/  jmethodID methodID,
                         /*[in]*/  jvalue *args, 
                         /*[out]*/ jobject* result,
                         /*[in]*/  ISecurityContext* ctx = NULL);
   
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
    JD_IMETHOD CallMethod(/*[in]*/  jd_jni_type ret_type,
                          /*[in]*/  jobject obj, 
                          /*[in]*/  jmethodID methodID,
                          /*[in]*/  jvalue *args, 
                          /*[out]*/ jvalue* result,
                          /*[in]*/  ISecurityContext* ctx = NULL);

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
    JD_IMETHOD CallNonvirtualMethod(/*[in]*/  jd_jni_type ret_type,
                                    /*[in]*/  jobject obj, 
                                    /*[in]*/  jclass clazz,
                                    /*[in]*/  jmethodID methodID,
                                    /*[in]*/  jvalue *args, 
                                    /*[out]*/ jvalue* result,
                                    /*[in]*/  ISecurityContext* ctx = NULL);

    /**
     * Get a field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param obj        -- Java object.
     * @param fieldID    -- field id
     * @param result     -- return field value
     * @param ctx        -- security context 
     */
    JD_IMETHOD GetField(/*[in]*/  jd_jni_type field_type,
                        /*[in]*/  jobject obj, 
                        /*[in]*/  jfieldID fieldID,
                        /*[out]*/ jvalue* result,
                        /*[in]*/  ISecurityContext* ctx = NULL);

    /**
     * Set a field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param obj        -- Java object.
     * @param fieldID    -- field id
     * @param val        -- field value to set
     * @param ctx        -- security context 
     */
    JD_IMETHOD SetField(/*[in]*/ jd_jni_type field_type,
                        /*[in]*/ jobject obj, 
                        /*[in]*/ jfieldID fieldID,
                        /*[in]*/ jvalue val,
                        /*[in]*/ ISecurityContext* ctx = NULL);

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
    JD_IMETHOD CallStaticMethod(/*[in]*/  jd_jni_type ret_type,
                                /*[in]*/  jclass clazz,
                                /*[in]*/  jmethodID methodID,
                                /*[in]*/  jvalue *args, 
                                /*[out]*/ jvalue* result,
                                /*[in]*/  ISecurityContext* ctx = NULL);

    /**
     * Get a static field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param clazz      -- Class object.
     * @param fieldID    -- field id
     * @param result     -- return field value
     * @param ctx        -- security context 
     */
    JD_IMETHOD GetStaticField(/*[in]*/  jd_jni_type field_type,
                              /*[in]*/  jclass clazz, 
                              /*[in]*/  jfieldID fieldID, 
                              /*[out]*/ jvalue* result,
                              /*[in]*/  ISecurityContext* ctx = NULL);


    /**
     * Set a static field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param clazz      -- Class object.
     * @param fieldID    -- field id
     * @param val        -- field value to set
     * @param ctx        -- security context 
     */
    JD_IMETHOD SetStaticField(/*[in]*/ jd_jni_type field_type,
                              /*[in]*/ jclass clazz, 
                              /*[in]*/ jfieldID fieldID,
                              /*[in]*/ jvalue val,
                              /*[in]*/ ISecurityContext* ctx = NULL);


    JD_IMETHOD GetVersion(/*[out]*/ jint* version);

    JD_IMETHOD DefineClass(/*[in]*/  const char* name,
                           /*[in]*/  jobject loader,
                           /*[in]*/  const jbyte *buf,
                           /*[in]*/  jsize len,
                           /*[out]*/ jclass* clazz);

    JD_IMETHOD FindClass(/*[in]*/  const char* name, 
                         /*[out]*/ jclass* clazz);

    JD_IMETHOD GetSuperclass(/*[in]*/  jclass sub,
                             /*[out]*/ jclass* super);

    JD_IMETHOD IsAssignableFrom(/*[in]*/  jclass sub,
                                /*[in]*/  jclass super,
                                /*[out]*/ jboolean* result);

    JD_IMETHOD Throw(/*[in]*/  jthrowable obj,
                     /*[out]*/ jint* result);

    JD_IMETHOD ThrowNew(/*[in]*/  jclass clazz,
                        /*[in]*/  const char *msg,
                        /*[out]*/ jint* result);

    JD_IMETHOD ExceptionOccurred(/*[out]*/ jthrowable* result);

    JD_IMETHOD ExceptionDescribe(void);

    JD_IMETHOD ExceptionClear(void);

    JD_IMETHOD FatalError(/*[in]*/ const char* msg);

    JD_IMETHOD NewGlobalRef(/*[in]*/  jobject lobj, 
                            /*[out]*/ jobject* result);

    JD_IMETHOD DeleteGlobalRef(/*[in]*/ jobject gref);

    JD_IMETHOD DeleteLocalRef(/*[in]*/ jobject obj);

    JD_IMETHOD IsSameObject(/*[in]*/  jobject obj1,
                            /*[in]*/  jobject obj2,
                            /*[out]*/ jboolean* result);

    JD_IMETHOD AllocObject(/*[in]*/  jclass clazz,
                           /*[out]*/ jobject* result);

    JD_IMETHOD GetObjectClass(/*[in]*/  jobject obj,
                              /*[out]*/ jclass* result);

    JD_IMETHOD IsInstanceOf(/*[in]*/  jobject obj,
                            /*[in]*/  jclass clazz,
                            /*[out]*/ jboolean* result);

    JD_IMETHOD GetMethodID(/*[in]*/  jclass clazz, 
                           /*[in]*/  const char* name,
                           /*[in]*/  const char* sig,
                           /*[out]*/ jmethodID* id);

    JD_IMETHOD GetFieldID(/*[in]*/  jclass clazz, 
                          /*[in]*/  const char* name,
                          /*[in]*/  const char* sig,
                          /*[out]*/ jfieldID* id);

    JD_IMETHOD GetStaticMethodID(/*[in]*/  jclass clazz, 
                                 /*[in]*/  const char* name,
                                 /*[in]*/  const char* sig,
                                 /*[out]*/ jmethodID* id);

    JD_IMETHOD GetStaticFieldID(/*[in]*/  jclass clazz, 
                                /*[in]*/  const char* name,
                                /*[in]*/  const char* sig,
                                /*[out]*/ jfieldID* id);

    JD_IMETHOD NewString(/*[in]*/  const jchar* unicode,
                         /*[in]*/  jsize len,
                         /*[out]*/ jstring* result);

    JD_IMETHOD GetStringLength(/*[in]*/  jstring str,
                               /*[out]*/ jsize* result);
    
    JD_IMETHOD GetStringChars(/*[in]*/  jstring str,
                              /*[in]*/  jboolean *isCopy,
                              /*[out]*/ const jchar** result);

    JD_IMETHOD ReleaseStringChars(/*[in]*/  jstring str,
                                  /*[in]*/  const jchar *chars);

    JD_IMETHOD NewStringUTF(/*[in]*/  const char *utf,
                            /*[out]*/ jstring* result);

    JD_IMETHOD GetStringUTFLength(/*[in]*/  jstring str,
                                  /*[out]*/ jsize* result);
    
    JD_IMETHOD GetStringUTFChars(/*[in]*/  jstring str,
                                 /*[in]*/  jboolean *isCopy,
                                 /*[out]*/ const char** result);

    JD_IMETHOD ReleaseStringUTFChars(/*[in]*/  jstring str,
                                     /*[in]*/  const char *chars);

    JD_IMETHOD GetArrayLength(/*[in]*/  jarray array,
                              /*[out]*/ jsize* result);

    JD_IMETHOD GetObjectArrayElement(/*[in]*/  jobjectArray array,
                                     /*[in]*/  jsize index,
                                     /*[out]*/ jobject* result);

    JD_IMETHOD SetObjectArrayElement(/*[in]*/  jobjectArray array,
                                     /*[in]*/  jsize index,
                                     /*[in]*/  jobject val);

    JD_IMETHOD NewObjectArray(/*[in]*/  jsize len,
    					      /*[in]*/  jclass clazz,
                              /*[in]*/  jobject init,
                              /*[out]*/ jobjectArray* result);

    JD_IMETHOD NewArray(/*[in]*/ jd_jni_type element_type,
                        /*[in]*/  jsize len,
                        /*[out]*/ jarray* result);

    JD_IMETHOD GetArrayElements(/*[in]*/  jd_jni_type element_type,
                                /*[in]*/  jarray array,
                                /*[in]*/  jboolean *isCopy,
                                /*[out]*/ void* result);

    JD_IMETHOD ReleaseArrayElements(/*[in]*/ jd_jni_type element_type,
                                    /*[in]*/ jarray array,
                                    /*[in]*/ void *elems,
                                    /*[in]*/ jint mode);

    JD_IMETHOD GetArrayRegion(/*[in]*/  jd_jni_type element_type,
                              /*[in]*/  jarray array,
                              /*[in]*/  jsize start,
                              /*[in]*/  jsize len,
                              /*[out]*/ void* buf);

    JD_IMETHOD SetArrayRegion(/*[in]*/  jd_jni_type element_type,
                              /*[in]*/  jarray array,
                              /*[in]*/  jsize start,
                              /*[in]*/  jsize len,
                              /*[in]*/  void* buf);

    JD_IMETHOD RegisterNatives(/*[in]*/  jclass clazz,
                               /*[in]*/  const JNINativeMethod *methods,
                               /*[in]*/  jint nMethods,
                               /*[out]*/ jint* result);

    JD_IMETHOD UnregisterNatives(/*[in]*/  jclass clazz,
                                 /*[out]*/ jint* result);

    JD_IMETHOD MonitorEnter(/*[in]*/  jobject obj,
                            /*[out]*/ jint* result);

    JD_IMETHOD MonitorExit(/*[in]*/  jobject obj,
                           /*[out]*/ jint* result);

    JD_IMETHOD GetJavaVM(/*[in]*/  JavaVM **vm,
                         /*[out]*/ jint* result);
                         
    ////////////////////////////////////////////////////////////////////////////
    // from ISecureJNI:

    CSecureJNIEnv(ISupports *aOuter, RemoteJNIEnv* env);
    virtual ~CSecureJNIEnv(void);

protected:

    JD_IMETHOD Initialize();

    RemoteJNIEnv*     m_env;

};

#endif // CSecureJNIEnv_h___
