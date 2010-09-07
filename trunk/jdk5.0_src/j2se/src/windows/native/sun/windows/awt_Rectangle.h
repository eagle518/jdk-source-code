/*
 * @(#)awt_Rectangle.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

