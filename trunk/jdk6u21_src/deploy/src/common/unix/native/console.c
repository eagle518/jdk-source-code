/*
 * @(#)console.c	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef linux
/* To pick up RTLD_GLOBAL, which is not in the POSIX spec */
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

static void (*jws_JVM_DumpAllStacks)(JNIEnv *env, jclass unused) = NULL;

/* Dump all the thread stacks by calling JVM function */
JNIEXPORT jstring JNICALL Java_com_sun_deploy_util_ConsoleHelper_dumpAllStacksImpl
 (JNIEnv * env, jclass clazz) {
    jstring str = NULL;
    char* pszConsoleOutput = NULL;
    char* tmpFile = NULL;
    struct stat buf;
    int sfildes, tfildes;
    int len;

    /* Open a tmp file to record thread info */
    tmpFile = tmpnam(NULL);
    tfildes = open(tmpFile, O_CREAT|O_RDWR, 0666);
    if (unlink(tmpFile) == -1)
        return NULL;

    /* Duplicate a standard file descriptor */
    sfildes = dup(1);

    /* file descriptor 1 point to tfildes */
    dup2(tfildes, 1);

    /* Trigger thread dump */
    if (jws_JVM_DumpAllStacks == NULL) {
        jws_JVM_DumpAllStacks = (void (*)(JNIEnv *env, jclass unused))dlsym(RTLD_DEFAULT, "JVM_DumpAllStacks");
        if (jws_JVM_DumpAllStacks == NULL) {
            return NULL;
        }
    }

    jws_JVM_DumpAllStacks(env, NULL);
 
    /* file descriptor 1 point back to sfildes */
    dup2(sfildes, 1);

    /* Move the file pointer to the begining */
    lseek(tfildes, 0, SEEK_SET);

    /* Get the file size */
    fstat(tfildes, &buf);
    len = buf.st_size;

    /* Read the content in tmp file into output buffer */
    if (len > 0)
    {
       pszConsoleOutput = (char *) malloc(len * sizeof(char));
       read(tfildes, pszConsoleOutput, len);
       pszConsoleOutput[len] = '\0';
    }

    /* Display the thread output in Java Console */
    if (pszConsoleOutput != NULL)
       str = (*env)->NewStringUTF(env, pszConsoleOutput);

    /* Release memory */
    free(pszConsoleOutput);

    close(tfildes);

    return str;
}

JNIEXPORT jstring JNICALL Java_com_sun_deploy_util_ConsoleHelper_preMustangDumpAllStacksImpl
 (JNIEnv * env, jclass clazz) {

  /* unix implementation is the same in all cases */
  return Java_com_sun_deploy_util_ConsoleHelper_dumpAllStacksImpl(env, clazz);
}
