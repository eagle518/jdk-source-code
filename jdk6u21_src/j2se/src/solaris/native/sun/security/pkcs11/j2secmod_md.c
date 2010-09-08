/*
 * @(#)j2secmod_md.c	1.6 10/04/08
 *
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <dlfcn.h>
#include <link.h>

#include <jni_util.h>
 
#include "j2secmod.h"

void *findFunction(JNIEnv *env, jlong jHandle, const char *functionName) {
    void *hModule = (void*)jHandle;
    void *fAddress = dlsym(hModule, functionName);
    if (fAddress == NULL) {
	char errorMessage[256];
	snprintf(errorMessage, sizeof(errorMessage), "Symbol not found: %s", functionName);
	JNU_ThrowNullPointerException(env, errorMessage);
	return NULL;
    }
    return fAddress;
}

JNIEXPORT jlong JNICALL Java_sun_security_pkcs11_Secmod_nssGetLibraryHandle
  (JNIEnv *env, jclass thisClass, jstring jLibName)
{
    const char *libName = (*env)->GetStringUTFChars(env, jLibName, NULL);
    // look up existing handle only, do not load
    void *hModule = dlopen(libName, RTLD_NOLOAD);
    dprintf2("-handle for %s: %u\n", libName, hModule);
    (*env)->ReleaseStringUTFChars(env, jLibName, libName);
    return (jlong)hModule;
}

JNIEXPORT jlong JNICALL Java_sun_security_pkcs11_Secmod_nssLoadLibrary
  (JNIEnv *env, jclass thisClass, jstring jLibName)
{
    void *hModule;
    const char *libName = (*env)->GetStringUTFChars(env, jLibName, NULL);

    dprintf1("-lib %s\n", libName);
    hModule = dlopen(libName, RTLD_LAZY);
    (*env)->ReleaseStringUTFChars(env, jLibName, libName);
    dprintf2("-handle: %u (0X%X)\n", hModule, hModule);

    if (hModule == NULL) {
	JNU_ThrowIOException(env, dlerror());
	return 0;
    }
    
    return (jlong)hModule;
}

