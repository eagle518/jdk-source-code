/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__CSecureEnv__)
#define __CSecureEnv__

#include "nsplugin.h"
#include "nsISecureEnv.h"

class CSecureEnv : public nsISecureEnv {
public:

    NS_DECL_ISUPPORTS


    NS_IMETHOD NewObject(/*[in]*/  jclass clazz, 
                         /*[in]*/  jmethodID methodID,
                         /*[in]*/  jvalue *args, 
                         /*[out]*/ jobject* result,
                         /*[in]*/  nsISecurityContext* ctx = NULL) = 0;
   
    NS_IMETHOD CallMethod(/*[in]*/  jni_type type,
                          /*[in]*/  jobject obj, 
                          /*[in]*/  jmethodID methodID,
                          /*[in]*/  jvalue *args, 
                          /*[out]*/ jvalue* result,
                          /*[in]*/  nsISecurityContext* ctx = NULL) = 0;

    NS_IMETHOD CallNonvirtualMethod(/*[in]*/  jni_type type,
                                    /*[in]*/  jobject obj, 
                                    /*[in]*/  jclass clazz,
                                    /*[in]*/  jmethodID methodID,
                                    /*[in]*/  jvalue *args, 
                                    /*[out]*/ jvalue* result,
                                    /*[in]*/  nsISecurityContext* ctx = NULL) = 0;

    NS_IMETHOD GetField(/*[in]*/  jni_type type,
                        /*[in]*/  jobject obj, 
                        /*[in]*/  jfieldID fieldID,
                        /*[out]*/ jvalue* result,
                        /*[in]*/  nsISecurityContext* ctx = NULL) = 0;

    NS_IMETHOD SetField(/*[in]*/ jni_type type,
                        /*[in]*/ jobject obj, 
                        /*[in]*/ jfieldID fieldID,
                        /*[in]*/ jvalue val,
                        /*[in]*/ nsISecurityContext* ctx = NULL) = 0;

    NS_IMETHOD CallStaticMethod(/*[in]*/  jni_type type,
                                /*[in]*/  jclass clazz,
                                /*[in]*/  jmethodID methodID,
                                /*[in]*/  jvalue *args, 
                                /*[out]*/ jvalue* result,
                                /*[in]*/  nsISecurityContext* ctx = NULL) = 0;

    NS_IMETHOD GetStaticField(/*[in]*/  jni_type type,
                              /*[in]*/  jclass clazz, 
                              /*[in]*/  jfieldID fieldID, 
                              /*[out]*/ jvalue* result,
                              /*[in]*/  nsISecurityContext* ctx = NULL) = 0;


    NS_IMETHOD SetStaticField(/*[in]*/ jni_type type,
                              /*[in]*/ jclass clazz, 
                              /*[in]*/ jfieldID fieldID,
                              /*[in]*/ jvalue val,
                              /*[in]*/ nsISecurityContext* ctx = NULL) = 0;


    NS_IMETHOD GetVersion(/*[out]*/ jint* version) = 0;

    NS_IMETHOD DefineClass(/*[in]*/  const char* name,
                           /*[in]*/  jobject loader,
                           /*[in]*/  const jbyte *buf,
                           /*[in]*/  jsize len,
                           /*[out]*/ jclass* clazz) = 0;

    NS_IMETHOD FindClass(/*[in]*/  const char* name, 
                         /*[out]*/ jclass* clazz) = 0;

    NS_IMETHOD GetSuperclass(/*[in]*/  jclass sub,
                             /*[out]*/ jclass* super) = 0;

    NS_IMETHOD IsAssignableFrom(/*[in]*/  jclass sub,
                                /*[in]*/  jclass super,
                                /*[out]*/ jboolean* result) = 0;

    NS_IMETHOD Throw(/*[in]*/  jthrowable obj,
                     /*[out]*/ jint* result) = 0;

    NS_IMETHOD ThrowNew(/*[in]*/  jclass clazz,
                        /*[in]*/  const char *msg,
                        /*[out]*/ jint* result) = 0;

    NS_IMETHOD ExceptionOccurred(/*[out]*/ jthrowable* result) = 0;

    NS_IMETHOD ExceptionDescribe(void) = 0;

    NS_IMETHOD ExceptionClear(void) = 0;

    NS_IMETHOD FatalError(/*[in]*/ const char* msg) = 0;

    NS_IMETHOD NewGlobalRef(/*[in]*/  jobject lobj, 
                            /*[out]*/ jobject* result) = 0;

    NS_IMETHOD DeleteGlobalRef(/*[in]*/ jobject gref) = 0;

    NS_IMETHOD DeleteLocalRef(/*[in]*/ jobject obj) = 0;

    NS_IMETHOD IsSameObject(/*[in]*/  jobject obj1,
                            /*[in]*/  jobject obj2,
                            /*[out]*/ jboolean* result) = 0;

    NS_IMETHOD AllocObject(/*[in]*/  jclass clazz,
                           /*[out]*/ jobject* result) = 0;

    NS_IMETHOD GetObjectClass(/*[in]*/  jobject obj,
                              /*[out]*/ jclass* result) = 0;

    NS_IMETHOD IsInstanceOf(/*[in]*/  jobject obj,
                            /*[in]*/  jclass clazz,
                            /*[out]*/ jboolean* result) = 0;

    NS_IMETHOD GetMethodID(/*[in]*/  jclass clazz, 
                           /*[in]*/  const char* name,
                           /*[in]*/  const char* sig,
                           /*[out]*/ jmethodID* id) = 0;

    NS_IMETHOD GetFieldID(/*[in]*/  jclass clazz, 
                          /*[in]*/  const char* name,
                          /*[in]*/  const char* sig,
                          /*[out]*/ jfieldID* id) = 0;

    NS_IMETHOD GetStaticMethodID(/*[in]*/  jclass clazz, 
                                 /*[in]*/  const char* name,
                                 /*[in]*/  const char* sig,
                                 /*[out]*/ jmethodID* id) = 0;

    NS_IMETHOD GetStaticFieldID(/*[in]*/  jclass clazz, 
                                /*[in]*/  const char* name,
                                /*[in]*/  const char* sig,
                                /*[out]*/ jfieldID* id) = 0;

    NS_IMETHOD NewString(/*[in]*/  const jchar* unicode,
                         /*[in]*/  jsize len,
                         /*[out]*/ jstring* result) = 0;

    NS_IMETHOD GetStringLength(/*[in]*/  jstring str,
                               /*[out]*/ jsize* result) = 0;
    
    NS_IMETHOD GetStringChars(/*[in]*/  jstring str,
                              /*[in]*/  jboolean *isCopy,
                              /*[out]*/ const jchar** result) = 0;

    NS_IMETHOD ReleaseStringChars(/*[in]*/  jstring str,
                                  /*[in]*/  const jchar *chars) = 0;

    NS_IMETHOD NewStringUTF(/*[in]*/  const char *utf,
                            /*[out]*/ jstring* result) = 0;

    NS_IMETHOD GetStringUTFLength(/*[in]*/  jstring str,
                                  /*[out]*/ jsize* result) = 0;
    
    NS_IMETHOD GetStringUTFChars(/*[in]*/  jstring str,
                                 /*[in]*/  jboolean *isCopy,
                                 /*[out]*/ const char** result) = 0;

    NS_IMETHOD ReleaseStringUTFChars(/*[in]*/  jstring str,
                                     /*[in]*/  const char *chars) = 0;

    NS_IMETHOD GetArrayLength(/*[in]*/  jarray array,
                              /*[out]*/ jsize* result) = 0;

    NS_IMETHOD NewObjectArray(/*[in]*/  jsize len,
    					/*[in]*/  jclass clazz,
                        /*[in]*/  jobject init,
                        /*[out]*/ jobjectArray* result) = 0;

    NS_IMETHOD GetObjectArrayElement(/*[in]*/  jobjectArray array,
                                     /*[in]*/  jsize index,
                                     /*[out]*/ jobject* result) = 0;

    NS_IMETHOD SetObjectArrayElement(/*[in]*/  jobjectArray array,
                                     /*[in]*/  jsize index,
                                     /*[in]*/  jobject val) = 0;

    NS_IMETHOD NewArray(/*[in]*/ jni_type element_type,
                        /*[in]*/  jsize len,
                        /*[out]*/ jarray* result) = 0;

    NS_IMETHOD GetArrayElements(/*[in]*/  jni_type type,
                                /*[in]*/  jarray array,
                                /*[in]*/  jboolean *isCopy,
                                /*[out]*/ void* result) = 0;

    NS_IMETHOD ReleaseArrayElements(/*[in]*/ jni_type type,
                                    /*[in]*/ jarray array,
                                    /*[in]*/ void *elems,
                                    /*[in]*/ jint mode) = 0;

    NS_IMETHOD GetArrayRegion(/*[in]*/  jni_type type,
                              /*[in]*/  jarray array,
                              /*[in]*/  jsize start,
                              /*[in]*/  jsize len,
                              /*[out]*/ void* buf) = 0;

    NS_IMETHOD SetArrayRegion(/*[in]*/  jni_type type,
                              /*[in]*/  jarray array,
                              /*[in]*/  jsize start,
                              /*[in]*/  jsize len,
                              /*[in]*/  void* buf) = 0;

    NS_IMETHOD RegisterNatives(/*[in]*/  jclass clazz,
                               /*[in]*/  const JNINativeMethod *methods,
                               /*[in]*/  jint nMethods,
                               /*[out]*/ jint* result) = 0;

    NS_IMETHOD UnregisterNatives(/*[in]*/  jclass clazz,
                                 /*[out]*/ jint* result) = 0;

    NS_IMETHOD MonitorEnter(/*[in]*/  jobject obj,
                            /*[out]*/ jint* result) = 0;

    NS_IMETHOD MonitorExit(/*[in]*/  jobject obj,
                           /*[out]*/ jint* result) = 0;

    NS_IMETHOD GetJavaVM(/*[in]*/  JavaVM **vm,
                         /*[out]*/ jint* result) = 0;

    // constructors and destructor
    CSecureEnv();
    virtual ~CSecureEnv();

private:

};

#endif
