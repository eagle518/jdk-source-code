/*
 * @(#)j2secmod.c	1.7 10/04/08
 *
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define SECMOD_DEBUG

#include "j2secmod.h"


JNIEXPORT jboolean JNICALL Java_sun_security_pkcs11_Secmod_nssVersionCheck
  (JNIEnv *env, jclass thisClass, jlong jHandle, jstring jVersion)
{
    const char *requiredVersion = (*env)->GetStringUTFChars(env, jVersion, NULL);
    int res;
    FPTR_VersionCheck versionCheck =
	(FPTR_VersionCheck)findFunction(env, jHandle, "NSS_VersionCheck");

    if (versionCheck == NULL) {
	return JNI_FALSE;
    }

    res = versionCheck(requiredVersion);
    dprintf2("-version >=%s: %d\n", requiredVersion, res);
    (*env)->ReleaseStringUTFChars(env, jVersion, requiredVersion);

    return (res == 0) ? JNI_FALSE : JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_sun_security_pkcs11_Secmod_nssInit
  (JNIEnv *env, jclass thisClass, jstring jFunctionName, jlong jHandle, jstring jConfigDir)
{
    const char *functionName = (*env)->GetStringUTFChars(env, jFunctionName, NULL);
    const char *configDir = (jConfigDir == NULL) ? NULL : (*env)->GetStringUTFChars(env, jConfigDir, NULL);
    FPTR_Init init = (FPTR_Init)findFunction(env, jHandle, functionName);
    int res;

    (*env)->ReleaseStringUTFChars(env, jFunctionName, functionName);
    if (init == NULL) {
	return JNI_FALSE;
    }

    res = init(configDir);
    if (configDir != NULL) {
	(*env)->ReleaseStringUTFChars(env, jConfigDir, configDir);
    }
    dprintf1("-res: %d\n", res);

    return (res == 0) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jobject JNICALL Java_sun_security_pkcs11_Secmod_nssGetModuleList
  (JNIEnv *env, jclass thisClass, jlong jHandle)
{
    FPTR_GetDBModuleList getModuleList =
	(FPTR_GetDBModuleList)findFunction(env, jHandle, "SECMOD_GetDefaultModuleList");

    SECMODModuleList *list;
    SECMODModule *module;
    jclass jListClass, jModuleClass;
    jobject jList, jModule;
    jmethodID jListConstructor, jAdd, jModuleConstructor;
    jstring jCommonName, jDllName;
    jboolean jFIPS;
    jint i;

    if (getModuleList == NULL) {
	dprintf("-getmodulelist function not found\n");
	return NULL;
    }
    list = getModuleList();
    if (list == NULL) {
	dprintf("-module list is null\n");
	return NULL;
    }

    jListClass = (*env)->FindClass(env, "java/util/ArrayList");
    jListConstructor = (*env)->GetMethodID(env, jListClass, "<init>", "()V");
    jAdd = (*env)->GetMethodID(env, jListClass, "add", "(Ljava/lang/Object;)Z");
    jList = (*env)->NewObject(env, jListClass, jListConstructor);

    jModuleClass = (*env)->FindClass(env, "sun/security/pkcs11/Secmod$Module");
    jModuleConstructor = (*env)->GetMethodID
	(env, jModuleClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;ZI)V");

    while (list != NULL) {
	module = list->module;
	// assert module != null
	dprintf1("-commonname: %s\n", module->commonName);
	dprintf1("-dllname: %s\n", (module->dllName != NULL) ? module->dllName : "NULL");
	dprintf1("-slots: %d\n", module->slotCount);
	dprintf1("-loaded: %d\n", module->loaded);
	dprintf1("-internal: %d\n", module->internal);
	dprintf1("-fips: %d\n", module->isFIPS);
	jCommonName = (*env)->NewStringUTF(env, module->commonName);
	if (module->dllName == NULL) {
	    jDllName = NULL;
	} else {
	    jDllName = (*env)->NewStringUTF(env, module->dllName);
	}
	jFIPS = module->isFIPS; 
	for (i = 0; i < module->slotCount; i++ ) {
	    jModule = (*env)->NewObject(env, jModuleClass, jModuleConstructor, jDllName, jCommonName, jFIPS, i);
	    (*env)->CallVoidMethod(env, jList, jAdd, jModule);
	}
	list = list->next;
    }
    dprintf("-ok\n");

    return jList;
}
