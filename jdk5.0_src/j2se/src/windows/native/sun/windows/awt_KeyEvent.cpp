/*
 * @(#)awt_KeyEvent.cpp	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_KeyEvent.h"
#include "awt.h"

/************************************************************************
 * AwtKeyEvent fields
 */

jfieldID AwtKeyEvent::keyCodeID;
jfieldID AwtKeyEvent::keyCharID;

/************************************************************************
 * AwtKeyEvent native methods
 */

extern "C" {

JNIEXPORT void JNICALL 
Java_java_awt_event_KeyEvent_initIDs(JNIEnv *env, jclass cls) {
    TRY;

    AwtKeyEvent::keyCodeID = env->GetFieldID(cls, "keyCode", "I");
    AwtKeyEvent::keyCharID = env->GetFieldID(cls, "keyChar", "C");

    DASSERT(AwtKeyEvent::keyCodeID != NULL);
    DASSERT(AwtKeyEvent::keyCharID != NULL);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
