/*
 * @(#)commonRef.h	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_COMMONREF_H
#define JDWP_COMMONREF_H

#define NULL_OBJECT_ID ((jlong)0)

void commonRef_initialize(void);            
void commonRef_reset(void);            

jlong commonRef_refToID(jobject ref);
jobject commonRef_idToRef(jlong id);
void commonRef_idToRef_delete(JNIEnv *env, jobject ref);
jvmtiError commonRef_pin(jlong id);
jvmtiError commonRef_unpin(jlong id);
void commonRef_releaseMultiple(jlong id, jint refCount);
void commonRef_release(jlong id);
void commonRef_compact(void);

void commonRef_lock(void);
void commonRef_unlock(void);

#endif

