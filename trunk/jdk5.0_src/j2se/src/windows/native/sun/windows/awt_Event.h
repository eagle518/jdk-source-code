/*
 * @(#)awt_Event.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

