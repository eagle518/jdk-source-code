/*
 * @(#)config.c	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <jni.h>
#include "config.h"

JNIEXPORT jstring JNICALL Java_com_sun_deploy_config_UnixConfig_getEnv
  (JNIEnv *env, jclass cl, jstring name) {

  const char* nameStr = (*env)->GetStringUTFChars(env, name, JNI_FALSE);
  const char* envStr = getenv(nameStr);
  (*env)->ReleaseStringUTFChars(env, name, nameStr);

  if(envStr == NULL)
    return NULL;
  else {
    return (*env)->NewStringUTF(env, envStr);
  }
}

/**
 * If _POSIX_ARG_MAX is defined on this machine,
 * it is a guaranteed value.
 *
 * Otherwise we have to pick a safe one, posix violation.
 */
#ifdef _POSIX_ARG_MAX
    #define _THISMACHINE_ARG_MAX _POSIX_ARG_MAX
#else
    #define _THISMACHINE_ARG_MAX 2048
#endif

JNIEXPORT jint JNICALL Java_com_sun_deploy_config_UnixConfig_getPlatformMaxCommandLineLength
  (JNIEnv *env, jobject objHandle) {

  long res = sysconf(_SC_ARG_MAX);

  if ( res < 0 ) {
    /* sysconf doesn't know _SC_ARG_MAX, posix violation */
    res = _THISMACHINE_ARG_MAX;
  }
  if ( res < _THISMACHINE_ARG_MAX ) {
    /* sysconf returns < _POSIX_ARG_MAX, posix violation */
    res = _THISMACHINE_ARG_MAX;
  }
  return (jint) res;
}

JNIEXPORT jlong JNICALL Java_com_sun_deploy_config_UnixConfig_getPlatformPID
  (JNIEnv *env, jobject objHandle) {

  return (long) getpid();
}

