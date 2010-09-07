/*
 * @(#)awt_Panel.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/************************************************************************
 * AwtPanel class
 */

#ifndef AWT_PANEL_H
#define AWT_PANEL_H

#include <jni.h>
#include <jni_util.h>

class AwtPanel {
public:    
    static void* Restack(void * param);

    /* java.awt.Panel field ids */
    static jfieldID insets_ID;

};

#endif // AWT_PANEL_H

