/*
 * @(#)awt_Rectangle.h	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtRectangle class
 */

#ifndef AWT_RECTANGLE_H
#define AWT_RECTANGLE_H

#include <jni.h>
#include <jni_util.h>

class AwtRectangle {
public:

    /* java.awt.Rectangle field ids */
    static jfieldID xID;
    static jfieldID yID;
    static jfieldID widthID;
    static jfieldID heightID;

};

#endif // AWT_RECTANGLE_H

