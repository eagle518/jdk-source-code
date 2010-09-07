/*
 * @(#)awt_KeyEvent.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

};

#endif // AWT_KEYEVENT_H

