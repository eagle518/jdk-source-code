/*
 * @(#)stepControl.h	1.25 04/02/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_STEPCONTROL_H
#define JDWP_STEPCONTROL_H

#include "eventFilter.h"
#include "eventHandler.h"

typedef struct {
    /* Parameters */
    jint granularity;
    jint depth;

    /* State */
    jboolean pending;
    jboolean frameExited;    /* for depth == STEP_OVER or STEP_OUT */
    jboolean fromNative;
    jint fromStackDepth;     /* for all but STEP_INTO STEP_INSTRUCTION */
    jint fromLine;           /* for granularity == STEP_LINE */
    jmethodID method;	/* Where line table came from. */
    jvmtiLineNumberEntry *lineEntries;       /* STEP_LINE */
    jint lineEntryCount;     /* for granularity == STEP_LINE */

    HandlerNode *stepHandlerNode;
    HandlerNode *catchHandlerNode;
    HandlerNode *framePopHandlerNode;
    HandlerNode *methodEnterHandlerNode;
} StepRequest;


void stepControl_initialize(void);
void stepControl_reset(void);

jboolean stepControl_handleStep(JNIEnv *env, jthread thread, 
				jclass clazz, jmethodID method);

jvmtiError stepControl_beginStep(jthread thread, jint size, jint depth,
                           HandlerNode *node);
jvmtiError stepControl_endStep(jthread thread);

void stepControl_clearRequest(jthread thread, StepRequest *step);
void stepControl_resetRequest(jthread thread);

void stepControl_lock(void);
void stepControl_unlock(void);

#endif

