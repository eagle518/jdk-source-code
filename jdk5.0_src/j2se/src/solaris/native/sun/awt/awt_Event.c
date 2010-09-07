/*
 * @(#)awt_Event.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/***
 *** THIS IMPLEMENTS ONLY THE OBSOLETE java.awt.Event CLASS! SEE
 *** awt_AWTEvent.[ch] FOR THE NEWER EVENT CLASSES.
 ***
 ***/
#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "java_awt_Event.h"
#include "jni_util.h"

#include "awt_Event.h"

struct EventIDs eventIDs;

JNIEXPORT void JNICALL
Java_java_awt_Event_initIDs(JNIEnv *env, jclass cls)
{
    eventIDs.data = (*env)->GetFieldID(env, cls, "data", "J");
    eventIDs.consumed = (*env)->GetFieldID(env, cls, "consumed", "Z");
    eventIDs.id = (*env)->GetFieldID(env, cls, "id", "I");
}

