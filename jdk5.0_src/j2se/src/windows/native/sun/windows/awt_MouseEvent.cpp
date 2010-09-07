/*
 * @(#)awt_MouseEvent.cpp	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_MouseEvent.h"
#include "awt.h"

/************************************************************************
 * AwtMouseEvent fields
 */

jfieldID AwtMouseEvent::xID;
jfieldID AwtMouseEvent::yID;
jfieldID AwtMouseEvent::buttonID;

/************************************************************************
 * AwtMouseEvent native methods
 */

extern "C" {

JNIEXPORT void JNICALL 
Java_java_awt_event_MouseEvent_initIDs(JNIEnv *env, jclass cls) {
    TRY;

    AwtMouseEvent::xID = env->GetFieldID(cls, "x", "I");
    AwtMouseEvent::yID = env->GetFieldID(cls, "y", "I");
    AwtMouseEvent::buttonID = env->GetFieldID(cls, "button", "I");

    DASSERT(AwtMouseEvent::xID != NULL);
    DASSERT(AwtMouseEvent::yID != NULL);
    DASSERT(AwtMouseEvent::buttonID != NULL);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
