/*
 * @(#)awt_AWTEvent.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Implements the native code for the java.awt.AWTEvent class
 * and all of the classes in the java.awt.event package.
 *
 * THIS FILE DOES NOT IMPLEMENT ANY OF THE OBSOLETE java.awt.Event
 * CLASS. SEE awt_Event.[ch] FOR THAT CLASS' IMPLEMENTATION.
 */
#ifndef _AWT_AWTEVENT_H_
#define _AWT_AWTEVENT_H_

#include "jni_util.h"

struct AWTEventIDs {
  jfieldID bdata;
  jfieldID consumed;
  jfieldID id;
};

struct InputEventIDs {
  jfieldID modifiers;
};

struct KeyEventIDs {
  jfieldID keyCode;
  jfieldID keyChar;
};

extern struct AWTEventIDs awtEventIDs;
extern struct InputEventIDs inputEventIDs;
extern struct KeyEventIDs keyEventIDs;

#endif /* _AWT_AWTEVENT_H_ */
