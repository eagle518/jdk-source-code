/*
 * @(#)CNSAdapter_SecureJNIEnv.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecureJNIEnv.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecureJNIEnv.h is Declaration of adapter for nsISecureJNIEnv
// 
//
#ifndef __CNSAdapter_SecureJNIEnv_H_
#define __CNSAdapter_SecureJNIEnv_H_

#include "nsISecureEnv.h"
#include "nscore.h"

class ISecureEnv;
class CNSAdapter_SecureJNIEnv : public nsISecureEnv
{
public:

    ////////////////////////////////////////////////////////////////////////////
    // from nsISupports 

    NS_DECL_ISUPPORTS

    ////////////////////////////////////////////////////////////////////////////
    // from nsISecureEnv:


    /**
     * Create new Java object in LiveConnect.
     *
     * @param clazz      -- Java Class object.
     * @param methodID   -- Method id
     * @param args       -- arguments for invoking the constructor.
     * @param result     -- return new Java object.
     * @param ctx        -- security context 
     */
    NS_IMETHOD NewObject(/*[in]*/  jclass clazz, 
                         /*[in]*/  jmethodID methodID,
                         /*[in]*/  jvalue *args, 
                         /*[out]*/ jobject* result,
                         /*[in]*/  nsISecurityContext* ctx = NULL);
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
    NS_IMETHOD CallMethod(/*[in]*/  jni_type ret_type,
                          /*[in]*/  jobject obj, 
                          /*[in]*/  jmethodID methodID,
                          /*[in]*/  jvalue *args, 
                          /*[out]*/ jvalue* result,
                          /*[in]*/  nsISecurityContext* ctx = NULL);
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
    NS_IMETHOD CallNonvirtualMethod(/*[in]*/  jni_type ret_type,
                                    /*[in]*/  jobject obj, 
                                    /*[in]*/  jclass clazz,
                                    /*[in]*/  jmethodID methodID,
                                    /*[in]*/  jvalue *args, 
                                    /*[out]*/ jvalue* result,
                                    /*[in]*/  nsISecurityContext* ctx = NULL);
    /**
     * Get a field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param obj        -- Java object.
     * @param fieldID    -- field id
     * @param result     -- return field value
     * @param ctx        -- security context 
     */
    NS_IMETHOD GetField(/*[in]*/  jni_type field_type,
                        /*[in]*/  jobject obj, 
                        /*[in]*/  jfieldID fieldID,
                        /*[out]*/ jvalue* result,
                        /*[in]*/  nsISecurityContext* ctx = NULL);

    /**
     * Set a field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param obj        -- Java object.
     * @param fieldID    -- field id
     * @param val        -- field value to set
     * @param ctx        -- security context 
     */
    NS_IMETHOD SetField(/*[in]*/ jni_type field_type,
                        /*[in]*/ jobject obj, 
                        /*[in]*/ jfieldID fieldID,
                        /*[in]*/ jvalue val,
                        /*[in]*/ nsISecurityContext* ctx = NULL);
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
    NS_IMETHOD CallStaticMethod(/*[in]*/  jni_type ret_type,
                                /*[in]*/  jclass clazz,
                                /*[in]*/  jmethodID methodID,
                                /*[in]*/  jvalue *args, 
                                /*[out]*/ jvalue* result,
                                /*[in]*/  nsISecurityContext* ctx = NULL);
    /**
     * Get a static field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param clazz      -- Class object.
     * @param fieldID    -- field id
     * @param result     -- return field value
     * @param ctx        -- security context 
     */
    NS_IMETHOD GetStaticField(/*[in]*/  jni_type field_type,
                              /*[in]*/  jclass clazz, 
                              /*[in]*/  jfieldID fieldID, 
                              /*[out]*/ jvalue* result,
                              /*[in]*/  nsISecurityContext* ctx = NULL);

    /**
     * Set a static field on Java object in LiveConnect.
     *
     * @param type       -- Return type
     * @param clazz      -- Class object.
     * @param fieldID    -- field id
     * @param val        -- field value to set
     * @param ctx        -- security context 
     */
    NS_IMETHOD SetStaticField(/*[in]*/ jni_type field_type,
                              /*[in]*/ jclass clazz, 
                              /*[in]*/ jfieldID fieldID,
                              /*[in]*/ jvalue val,
                              /*[in]*/ nsISecurityContext* ctx = NULL);


    NS_IMETHOD GetVersion(/*[out]*/ jint* version);

    NS_IMETHOD DefineClass(/*[in]*/  const char* name,
                           /*[in]*/  jobject loader,
                           /*[in]*/  const jbyte *buf,
                           /*[in]*/  jsize len,
                           /*[out]*/ jclass* clazz);

    NS_IMETHOD FindClass(/*[in]*/  const char* name, 
                         /*[out]*/ jclass* clazz);

    NS_IMETHOD GetSuperclass(/*[in]*/  jclass sub,
                             /*[out]*/ jclass* super);

    NS_IMETHOD IsAssignableFrom(/*[in]*/  jclass sub,
                                /*[in]*/  jclass super,
                                /*[out]*/ jboolean* result);

    NS_IMETHOD Throw(/*[in]*/  jthrowable obj,
                     /*[out]*/ jint* result);

    NS_IMETHOD ThrowNew(/*[in]*/  jclass clazz,
                        /*[in]*/  const char *msg,
                        /*[out]*/ jint* result);
	
    NS_IMETHOD ExceptionOccurred(/*[out]*/ jthrowable* result);
	
    NS_IMETHOD ExceptionDescribe(void);
	
    NS_IMETHOD ExceptionClear(void);
	
    NS_IMETHOD FatalError(/*[in]*/ const char* msg);
	
    NS_IMETHOD NewGlobalRef(/*[in]*/  jobject lobj, 
                            /*[out]*/ jobject* result);
	
    NS_IMETHOD DeleteGlobalRef(/*[in]*/ jobject gref);
	
    NS_IMETHOD DeleteLocalRef(/*[in]*/ jobject obj);
	

    NS_IMETHOD IsSameObject(/*[in]*/  jobject obj1,
                            /*[in]*/  jobject obj2,
                            /*[out]*/ jboolean* result);
	
    NS_IMETHOD AllocObject(/*[in]*/  jclass clazz,
                           /*[out]*/ jobject* result);
	

    NS_IMETHOD GetObjectClass(/*[in]*/  jobject obj,
                              /*[out]*/ jclass* result);
	

    NS_IMETHOD IsInstanceOf(/*[in]*/  jobject obj,
                            /*[in]*/  jclass clazz,
                            /*[out]*/ jboolean* result);


    NS_IMETHOD GetMethodID(/*[in]*/  jclass clazz, 
                           /*[in]*/  const char* name,
                           /*[in]*/  const char* sig,
                           /*[out]*/ jmethodID* id);
	

    NS_IMETHOD GetFieldID(/*[in]*/  jclass clazz, 
                          /*[in]*/  const char* name,
                          /*[in]*/  const char* sig,
                          /*[out]*/ jfieldID* id);
	
    NS_IMETHOD GetStaticMethodID(/*[in]*/  jclass clazz, 
                                 /*[in]*/  const char* name,
                                 /*[in]*/  const char* sig,
                                 /*[out]*/ jmethodID* id);


    NS_IMETHOD GetStaticFieldID(/*[in]*/  jclass clazz, 
                                /*[in]*/  const char* name,
                                /*[in]*/  const char* sig,
                                /*[out]*/ jfieldID* id);
	

    NS_IMETHOD NewString(/*[in]*/  const jchar* unicode,
                         /*[in]*/  jsize len,
                         /*[out]*/ jstring* result);
	
    NS_IMETHOD GetStringLength(/*[in]*/  jstring str,
	                          /*[out]*/ jsize* result);
	
    
    NS_IMETHOD GetStringChars(/*[in]*/  jstring str,
                              /*[in]*/  jboolean *isCopy,
                              /*[out]*/ const jchar** result);
	
    NS_IMETHOD ReleaseStringChars(/*[in]*/  jstring str,
                                  /*[in]*/  const jchar *chars);
	
    NS_IMETHOD NewStringUTF(/*[in]*/  const char *utf,
                            /*[out]*/ jstring* result);
	

    NS_IMETHOD GetStringUTFLength(/*[in]*/  jstring str,
                                  /*[out]*/ jsize* result);
	
    
    NS_IMETHOD GetStringUTFChars(/*[in]*/  jstring str,
                                 /*[in]*/  jboolean *isCopy,
                                 /*[out]*/ const char** result);
	

    NS_IMETHOD ReleaseStringUTFChars(/*[in]*/  jstring str,
                                     /*[in]*/  const char *chars);
	

    NS_IMETHOD GetArrayLength(/*[in]*/  jarray array,
                              /*[out]*/ jsize* result);
	

    NS_IMETHOD GetObjectArrayElement(/*[in]*/  jobjectArray array,
                                     /*[in]*/  jsize index,
                                     /*[out]*/ jobject* result);
	

    NS_IMETHOD SetObjectArrayElement(/*[in]*/  jobjectArray array,
                                     /*[in]*/  jsize index,
                                     /*[in]*/  jobject val);
	

    NS_IMETHOD NewObjectArray(/*[in]*/  jsize len,
    			      /*[in]*/  jclass clazz,
                              /*[in]*/  jobject init,
                              /*[out]*/ jobjectArray* result);

	

    NS_IMETHOD NewArray(/*[in]*/  jni_type element_type,
                        /*[in]*/  jsize len,
                        /*[out]*/ jarray* result);
	

    NS_IMETHOD GetArrayElements(/*[in]*/  jni_type element_type,
                                /*[in]*/  jarray array,
                                /*[in]*/  jboolean *isCopy,
                                /*[out]*/ void* result);
	

    NS_IMETHOD ReleaseArrayElements(/*[in]*/ jni_type element_type,
                                    /*[in]*/ jarray array,
                                    /*[in]*/ void *elems,
                                    /*[in]*/ jint mode);
	

    NS_IMETHOD GetArrayRegion(/*[in]*/  jni_type element_type,
                              /*[in]*/  jarray array,
                              /*[in]*/  jsize start,
                              /*[in]*/  jsize len,
                              /*[out]*/ void* buf);
	

    NS_IMETHOD SetArrayRegion(/*[in]*/  jni_type element_type,
                              /*[in]*/  jarray array,
                              /*[in]*/  jsize start,
                              /*[in]*/  jsize len,
                              /*[in]*/  void* buf);
	

    NS_IMETHOD RegisterNatives(/*[in]*/  jclass clazz,
                               /*[in]*/  const JNINativeMethod *methods,
                               /*[in]*/  jint nMethods,
                               /*[out]*/ jint* result);
	

    NS_IMETHOD UnregisterNatives(/*[in]*/  jclass clazz,
                                 /*[out]*/ jint* result);
	

    NS_IMETHOD MonitorEnter(/*[in]*/  jobject obj,
                            /*[out]*/ jint* result);
	

    NS_IMETHOD MonitorExit(/*[in]*/  jobject obj,
                           /*[out]*/ jint* result);


    NS_IMETHOD GetJavaVM(/*[in]*/  JavaVM **vm,
                         /*[out]*/ jint* result);

    CNSAdapter_SecureJNIEnv(ISecureEnv* pSecureEnv);
    virtual ~CNSAdapter_SecureJNIEnv(void);
private:
    ISecureEnv* m_pSecureEnv;

    jd_jni_type TypeConvert(jni_type type);
	
};

#endif //__CNSAdapter_SecureJNIEnv_H_
