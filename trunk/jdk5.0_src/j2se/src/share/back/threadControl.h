/*
 * @(#)threadControl.h	1.46 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_THREADCONTROL_H
#define JDWP_THREADCONTROL_H

#include "stepControl.h"
#include "invoker.h"
#include "bag.h"

void threadControl_initialize(void);
void threadControl_reset(void);
void threadControl_detachInvokes(void);

void threadControl_onHook(void);
void threadControl_onConnect(void);
void threadControl_onDisconnect(void);

jvmtiError threadControl_popFrames(jthread thread, FrameNumber fnum);

struct bag *threadControl_onEventHandlerEntry(jbyte sessionID, 
		  EventIndex ei, jthread thread, jobject currentException);
void threadControl_onEventHandlerExit(EventIndex ei, jthread thread, struct bag *);


jvmtiError threadControl_suspendThread(jthread thread, jboolean deferred);
jvmtiError threadControl_resumeThread(jthread thread, jboolean do_unblock);
jvmtiError threadControl_suspendCount(jthread thread, jint *count);

jvmtiError threadControl_suspendAll(void);
jvmtiError threadControl_resumeAll(void);

StepRequest *threadControl_getStepRequest(jthread);
InvokeRequest *threadControl_getInvokeRequest(jthread);

jboolean threadControl_isDebugThread(jthread thread);
jvmtiError threadControl_addDebugThread(jthread thread);

jvmtiError threadControl_applicationThreadStatus(jthread thread, jdwpThreadStatus *pstatus, jint *suspendStatus);
jvmtiError threadControl_interrupt(jthread thread);
jvmtiError threadControl_stop(jthread thread, jobject throwable);

jvmtiError threadControl_setEventMode(jvmtiEventMode mode, EventIndex ei, jthread thread);
jvmtiEventMode threadControl_getInstructionStepMode(jthread thread);

jthread threadControl_currentThread(void);
void threadControl_setPendingInterrupt(jthread thread);
void threadControl_clearCLEInfo(JNIEnv *env, jthread thread);
jboolean threadControl_cmpCLEInfo(JNIEnv *env, jthread thread, jclass clazz,
                                  jmethodID method, jlocation location);
void threadControl_saveCLEInfo(JNIEnv *env, jthread thread, EventIndex ei,
                               jclass clazz, jmethodID method,
                               jlocation location);
jlong threadControl_getFrameGeneration(jthread thread);

#endif

