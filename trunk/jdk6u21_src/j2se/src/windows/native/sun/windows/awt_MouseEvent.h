/*
 * @(#)awt_MouseEvent.h	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtMouseEvent class
 */

#ifndef AWT_MOUSEEVENT_H
#define AWT_MOUSEEVENT_H

#include <jni.h>
#include <jni_util.h>

class AwtMouseEvent {
public:

    /* java.awt.MouseEvent field ids */
    static jfieldID xID;
    static jfieldID yID;
    static jfieldID buttonID;

};

#endif // AWT_MOUSEEVENT_H

