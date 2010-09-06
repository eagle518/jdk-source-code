/*
 * @(#)config.c	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
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
