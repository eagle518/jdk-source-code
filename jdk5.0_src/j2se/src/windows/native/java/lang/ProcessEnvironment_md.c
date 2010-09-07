/*
 * @(#)ProcessEnvironment_md.c	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * @author  Martin Buchholz
 * @version 1.5, 03/12/19
 * @since   1.5
 */
#include <stdlib.h>

#include "jni.h"
#include "jni_util.h"
#include <windows.h>

static jstring
environmentBlock9x(JNIEnv *env)
{
    int i;
    jmethodID String_init_ID =
	(*env)->GetMethodID(env, JNU_ClassString(env), "<init>", "([B)V");
    jbyteArray bytes;
    jbyte *blockA = (jbyte *) GetEnvironmentStringsA();
    if (blockA == NULL) {
	/* Both GetEnvironmentStringsW and GetEnvironmentStringsA
	 * failed.  Out of memory is our best guess.  */
	JNU_ThrowOutOfMemoryError(env, "GetEnvironmentStrings failed");
	return NULL;
    }

    /* Don't search for "\0\0", since an empty environment block may
       legitimately consist of a single "\0". */
    for (i = 0; blockA[i];)
	while (blockA[i++])
	    ;

    if ((bytes = (*env)->NewByteArray(env, i)) == NULL) return NULL;
    (*env)->SetByteArrayRegion(env, bytes, 0, i, blockA);
    FreeEnvironmentStringsA(blockA);
    return (*env)->NewObject(env, JNU_ClassString(env),
			     String_init_ID, bytes);
}

/* Returns a Windows style environment block, discarding final trailing NUL */
JNIEXPORT jstring JNICALL
Java_java_lang_ProcessEnvironment_environmentBlock(JNIEnv *env, jclass klass)
{
    int i;
    jstring envblock;
    jchar *blockW = (jchar *) GetEnvironmentStringsW();
    if (blockW == NULL)
	return environmentBlock9x(env);

    /* Don't search for "\u0000\u0000", since an empty environment
       block may legitimately consist of a single "\u0000".  */
    for (i = 0; blockW[i];)
	while (blockW[i++])
	    ;

    envblock = (*env)->NewString(env, blockW, i);
    FreeEnvironmentStringsW(blockW);
    return envblock;
}
