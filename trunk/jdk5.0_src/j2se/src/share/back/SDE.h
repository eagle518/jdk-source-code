/*
 * @(#)SDE.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_SDE_H
#define JDWP_SDE_H

void
convertLineNumberTable(JNIEnv *env, jclass clazz,
                       jint *entryCountPtr, 
                       jvmtiLineNumberEntry **tablePtr);

void
setGlobalStratumId(char *id);

#endif

