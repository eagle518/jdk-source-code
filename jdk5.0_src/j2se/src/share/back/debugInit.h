/*
 * @(#)debugInit.h	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_DEBUGINIT_H
#define JDWP_DEBUGINIT_H

void debugInit_waitInitComplete(void);
jboolean debugInit_isInitComplete(void);

/*
 * Access to debug options
 */
char *debugInit_launchOnInit(void);
jboolean debugInit_suspendOnInit(void);

void debugInit_reset(void);
void debugInit_exit(jvmtiError, const char *);
void forceExit(int);

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *, char *, void *); 
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *); 

#endif

