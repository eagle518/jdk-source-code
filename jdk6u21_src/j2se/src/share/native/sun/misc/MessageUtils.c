/*
 * @(#)MessageUtils.c	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <jni.h>
#include <jlong.h>
#include <stdio.h>
#include <jvm.h>

#include "sun_misc_MessageUtils.h"
 
static void 
printToFile(JNIEnv *env, jstring s, FILE *file)
{ 
    char *sConverted;
    int length;
    int i;
    const jchar *sAsArray;

    sAsArray = (*env)->GetStringChars(env, s, NULL); 
    length = (*env)->GetStringLength(env, s);
    sConverted = (char *) malloc(length + 1);
    for(i = 0; i < length; i++) {
	sConverted[i] = (0x7f & sAsArray[i]);
    }
    sConverted[length] = '\0';
    jio_fprintf(file, "%s", sConverted);
    (*env)->ReleaseStringChars(env, s, sAsArray);
    free(sConverted); 
}

JNIEXPORT void JNICALL
Java_sun_misc_MessageUtils_toStderr(JNIEnv *env, jclass cls, jstring s)
{
    printToFile(env, s, stderr);    
}

JNIEXPORT void JNICALL
Java_sun_misc_MessageUtils_toStdout(JNIEnv *env, jclass cls, jstring s)
{
    printToFile(env, s, stdout);
}

