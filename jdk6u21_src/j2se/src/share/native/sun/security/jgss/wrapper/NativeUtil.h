/*
 * @(#)NativeUtil.h	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include "gssapi.h"

#ifndef _Included_NATIVE_Util
#define _Included_NATIVE_Util
#ifdef __cplusplus
extern "C" {
#endif
  extern jint getJavaTime(OM_uint32);
  extern OM_uint32 getGSSTime(jint);
  extern void checkStatus(JNIEnv *, jobject, OM_uint32, OM_uint32, char*);
  extern jint checkTime(OM_uint32);
  extern void initGSSBuffer(JNIEnv *, jbyteArray, gss_buffer_t);
  extern void resetGSSBuffer(JNIEnv *, jbyteArray, gss_buffer_t);

  extern gss_OID newGSSOID(JNIEnv *, jobject);
  extern void deleteGSSOID(gss_OID);
  extern gss_OID_set newGSSOIDSet(JNIEnv *, gss_OID);
  extern void deleteGSSOIDSet(gss_OID_set);

  extern jbyteArray getJavaBuffer(JNIEnv *, gss_buffer_t);
  extern jstring getJavaString(JNIEnv *, gss_buffer_t);
  extern jobject getJavaOID(JNIEnv *, gss_OID);
  extern jobjectArray getJavaOIDArray(JNIEnv *, gss_OID_set);

  extern jstring getMinorMessage(JNIEnv *, jobject, OM_uint32);
  extern void debug(JNIEnv *, char *);
  extern int sameMech(JNIEnv *, gss_OID, gss_OID);

  JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *, void *);
  JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *, void *);

  extern char debugBuf[];

  extern jclass CLS_Object;
  extern jclass CLS_GSSNameElement;
  extern jclass CLS_GSSCredElement;
  extern jclass CLS_NativeGSSContext;
  extern jmethodID MID_MessageProp_getPrivacy;
  extern jmethodID MID_MessageProp_getQOP;
  extern jmethodID MID_MessageProp_setPrivacy;
  extern jmethodID MID_MessageProp_setQOP;
  extern jmethodID MID_MessageProp_setSupplementaryStates;
  extern jmethodID MID_ChannelBinding_getInitiatorAddr;
  extern jmethodID MID_ChannelBinding_getAcceptorAddr;
  extern jmethodID MID_ChannelBinding_getAppData;
  extern jmethodID MID_InetAddress_getAddr;
  extern jmethodID MID_GSSNameElement_ctor;
  extern jmethodID MID_GSSCredElement_ctor;
  extern jmethodID MID_NativeGSSContext_ctor;
  extern jfieldID FID_GSSLibStub_pMech;
  extern jfieldID FID_NativeGSSContext_pContext;
  extern jfieldID FID_NativeGSSContext_srcName;
  extern jfieldID FID_NativeGSSContext_targetName;
  extern jfieldID FID_NativeGSSContext_isInitiator;
  extern jfieldID FID_NativeGSSContext_isEstablished;
  extern jfieldID FID_NativeGSSContext_delegatedCred;
  extern jfieldID FID_NativeGSSContext_flags;
  extern jfieldID FID_NativeGSSContext_lifetime;
  extern jfieldID FID_NativeGSSContext_actualMech;
#ifdef __cplusplus
}
#endif
#endif
