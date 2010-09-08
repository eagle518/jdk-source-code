/*
 * @(#)awt_Event.h	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/***
 *** THIS IMPLEMENTS ONLY THE OBSOLETE java.awt.Event CLASS! SEE
 *** awt_AWTEvent.[ch] FOR THE NEWER EVENT CLASSES.
 ***
 ***/
#ifndef _AWT_EVENT_H_
#define _AWT_EVENT_H

#include "jni_util.h"

struct EventIDs {
  jfieldID data;
  jfieldID consumed;
  jfieldID id;
};

extern struct EventIDs eventIDs;

#endif /* _AWT_EVENT_H_ */

