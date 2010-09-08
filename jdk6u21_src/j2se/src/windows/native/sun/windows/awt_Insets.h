/*
 * @(#)awt_Insets.h	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtInsets class
 */

#ifndef AWT_INSETS_H
#define AWT_INSETS_H

#include <jni.h>
#include <jni_util.h>

class AwtInsets {
public:

    /* java.awt.Insets field ids */
    static jfieldID leftID;
    static jfieldID rightID;
    static jfieldID topID;
    static jfieldID bottomID;

};

#endif // AWT_INSETS_H

