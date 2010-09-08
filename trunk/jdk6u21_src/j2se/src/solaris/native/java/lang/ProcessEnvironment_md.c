/*
 * @(#)ProcessEnvironment_md.c	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * @author  Martin Buchholz
 * @version 1.6, 03/23/10
 * @since   1.5
 */

#include <stdlib.h>
#include <string.h>
#include "jni.h"
#include "jni_util.h"

JNIEXPORT jobjectArray JNICALL
Java_java_lang_ProcessEnvironment_environ(JNIEnv *env, jclass ign)
{
    /* This is one of the rare times it's more portable to declare an
     * external symbol explicitly, rather than via a system header.
     * The declaration is standardized as part of UNIX98, but there is
     * no standard (not even de-facto) header file where the
     * declaration is to be found.  See:
     * http://www.opengroup.org/onlinepubs/007908799/xbd/envvar.html */
    extern char ** environ; /* environ[i] looks like: VAR=VALUE\0 */

    jsize count = 0;
    jsize i, j;
    jobjectArray result;
    jclass byteArrCls = (*env)->FindClass(env, "[B");

    for (i = 0; environ[i]; i++) {
	/* Ignore corrupted environment variables */
	if (strchr(environ[i], '=') != NULL)
	    count++;
    }

    result = (*env)->NewObjectArray(env, 2*count, byteArrCls, 0);
    if (result == NULL) return NULL;

    for (i = 0, j = 0; environ[i]; i++) {
	const char * varEnd = strchr(environ[i], '=');
	/* Ignore corrupted environment variables */
	if (varEnd != NULL) {
	    jbyteArray var, val;
	    const char * valBeg = varEnd + 1;
	    jsize varLength = varEnd - environ[i];
	    jsize valLength = strlen(valBeg);
	    var = (*env)->NewByteArray(env, varLength);
	    if (var == NULL) return NULL;
	    val = (*env)->NewByteArray(env, valLength);
	    if (val == NULL) return NULL;
	    (*env)->SetByteArrayRegion(env, var, 0, varLength,
				       (jbyte*) environ[i]);
	    (*env)->SetByteArrayRegion(env, val, 0, valLength,
				       (jbyte*) valBeg);
	    (*env)->SetObjectArrayElement(env, result, 2*j  , var);
	    (*env)->SetObjectArrayElement(env, result, 2*j+1, val);
	    (*env)->DeleteLocalRef(env, var);
	    (*env)->DeleteLocalRef(env, val);
	    j++;
	}
    }

    return result;
}
