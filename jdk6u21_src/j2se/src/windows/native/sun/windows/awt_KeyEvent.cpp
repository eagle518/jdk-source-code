/*
 * @(#)awt_KeyEvent.cpp	1.13 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_KeyEvent.h"
#include "awt.h"

/************************************************************************
 * AwtKeyEvent fields
 */

jfieldID AwtKeyEvent::keyCodeID;
jfieldID AwtKeyEvent::keyCharID;
jfieldID AwtKeyEvent::rawCodeID;
jfieldID AwtKeyEvent::primaryLevelUnicodeID;
jfieldID AwtKeyEvent::scancodeID;

/************************************************************************
 * AwtKeyEvent native methods
 */

extern "C" {

JNIEXPORT void JNICALL 
Java_java_awt_event_KeyEvent_initIDs(JNIEnv *env, jclass cls) {
    TRY;

    AwtKeyEvent::keyCodeID = env->GetFieldID(cls, "keyCode", "I");
    AwtKeyEvent::keyCharID = env->GetFieldID(cls, "keyChar", "C");
    AwtKeyEvent::rawCodeID = env->GetFieldID(cls, "rawCode", "J");
    AwtKeyEvent::primaryLevelUnicodeID = env->GetFieldID(cls, "primaryLevelUnicode", "J");
    AwtKeyEvent::scancodeID = env->GetFieldID(cls, "scancode", "J");

    DASSERT(AwtKeyEvent::keyCodeID != NULL);
    DASSERT(AwtKeyEvent::keyCharID != NULL);
    DASSERT(AwtKeyEvent::rawCodeID != NULL);
    DASSERT(AwtKeyEvent::primaryLevelUnicodeID != NULL);
    DASSERT(AwtKeyEvent::scancodeID != NULL);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
