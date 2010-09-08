/*
 * @(#)awt_Event.h	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtEvent class
 */

#ifndef AWT_EVENT_H
#define AWT_EVENT_H

#include <jni.h>
#include <jni_util.h>

class AwtEvent {
public:

    /* java.awt.Event field ids */
    static jfieldID targetID;
    static jfieldID xID;
    static jfieldID yID;


};

#endif // AWT_EVENT_H

