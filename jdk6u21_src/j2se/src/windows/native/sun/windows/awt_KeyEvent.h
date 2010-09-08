/*
 * @(#)awt_KeyEvent.h	1.12 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtKeyEvent class
 */

#ifndef AWT_KEYEVENT_H
#define AWT_KEYEVENT_H

#include <jni.h>
#include <jni_util.h>

class AwtKeyEvent {
public:

    /* java.awt.KeyEvent field ids */
    static jfieldID keyCodeID;
    static jfieldID keyCharID;
    static jfieldID rawCodeID;
    static jfieldID primaryLevelUnicodeID;
    static jfieldID scancodeID;
};

#endif // AWT_KEYEVENT_H

