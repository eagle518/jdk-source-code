/*
 * @(#)awt_Container.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
    static jfieldID ncomponentsID;
    static jfieldID componentID;
    static jfieldID layoutMgrID;
    static jmethodID findComponentAtMID;

};

#endif // AWT_CONTAINER_H

