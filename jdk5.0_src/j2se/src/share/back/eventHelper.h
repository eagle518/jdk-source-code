/*
 * @(#)eventHelper.h	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_EVENTHELPER_H
#define JDWP_EVENTHELPER_H

#include "bag.h"
#include "invoker.h"

void eventHelper_initialize(jbyte sessionID);
void eventHelper_reset(jbyte sessionID);
struct bag *eventHelper_createEventBag(void);

void eventHelper_recordEvent(EventInfo *evinfo, jint id, 
                             jbyte suspendPolicy, struct bag *eventBag);
void eventHelper_recordClassUnload(jint id, char *signature, struct bag *eventBag);
void eventHelper_recordFrameEvent(jint id, jbyte suspendPolicy, EventIndex ei,
                                  jthread thread, jclass clazz, 
                                  jmethodID method, jlocation location,
                                  struct bag *eventBag);

jbyte eventHelper_reportEvents(jbyte sessionID, struct bag *eventBag);
void eventHelper_reportInvokeDone(jbyte sessionID, jthread thread);
void eventHelper_reportVMInit(JNIEnv *env, jbyte sessionID, jthread thread, jbyte suspendPolicy);
void eventHelper_suspendThread(jbyte sessionID, jthread thread);

void eventHelper_holdEvents(void);
void eventHelper_releaseEvents(void);

void eventHelper_lock(void);
void eventHelper_unlock(void);

/*
 * Private interface for coordinating between eventHelper.c: commandLoop()
 * and ThreadReferenceImpl.c: resume() and VirtualMachineImpl.c: resume().
 */
void unblockCommandLoop(void);

#endif

