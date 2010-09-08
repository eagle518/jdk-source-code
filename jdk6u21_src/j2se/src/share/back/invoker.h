/*
 * @(#)invoker.h	1.17 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_INVOKER_H
#define JDWP_INVOKER_H

/* Invoke types */

#define INVOKE_CONSTRUCTOR 1
#define INVOKE_STATIC      2
#define INVOKE_INSTANCE    3 

typedef struct InvokeRequest {
    jboolean pending;      /* Is an invoke requested? */
    jboolean started;      /* Is an invoke happening? */
    jboolean available;    /* Is the thread in an invokable state? */
    jboolean detached;     /* Has the requesting debugger detached? */
    jint id;
    /* Input */
    jbyte invokeType;
    jbyte options;
    jclass clazz;
    jmethodID method;
    jobject instance;    /* for INVOKE_INSTANCE only */
    jvalue *arguments;
    jint argumentCount;
    char *methodSignature;
    /* Output */
    jvalue returnValue;  /* if no exception, for all but INVOKE_CONSTRUCTOR */
    jobject exception;   /* NULL if no exception was thrown */
} InvokeRequest;


void invoker_initialize(void);
void invoker_reset(void);

void invoker_lock(void);
void invoker_unlock(void);

void invoker_enableInvokeRequests(jthread thread);
jvmtiError invoker_requestInvoke(jbyte invokeType, jbyte options, jint id,
                           jthread thread, jclass clazz, jmethodID method,
                           jobject instance, 
                           jvalue *arguments, jint argumentCount);
jboolean invoker_doInvoke(jthread thread);

void invoker_completeInvokeRequest(jthread thread);
jboolean invoker_isPending(jthread thread);
jboolean invoker_isEnabled(jthread thread);
void invoker_detach(InvokeRequest *request);

#endif

