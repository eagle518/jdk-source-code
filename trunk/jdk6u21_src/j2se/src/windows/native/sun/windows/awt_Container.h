/*
 * @(#)awt_Container.h	1.13 10/03/23
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtContainer class
 */

#ifndef AWT_CONTAINER_H
#define AWT_CONTAINER_H

#include <jni.h>
#include <jni_util.h>

class AwtContainer {
public:

    /* java.awt.Container field ids */
    static jfieldID layoutMgrID;
    static jmethodID findComponentAtMID;

};

#endif // AWT_CONTAINER_H

