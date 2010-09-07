/*
 * @(#)awt_Insets.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

