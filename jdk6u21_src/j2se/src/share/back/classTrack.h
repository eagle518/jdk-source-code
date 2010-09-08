/*
 * @(#)classTrack.h	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_CLASSTRACK_H
#define JDWP_CLASSTRACK_H

/* 
 * Called after class unloads have occurred.  
 * The signatures of classes which were unloaded are returned.
 */
struct bag *
classTrack_processUnloads(JNIEnv *env);

/*
 * Add a class to the prepared class hash table.
 */
void
classTrack_addPreparedClass(JNIEnv *env, jclass klass);

/*
 * Initialize class tracking.
 */
void
classTrack_initialize(JNIEnv *env);

/*
 * Reset class tracking.
 */
void
classTrack_reset(void);

#endif

