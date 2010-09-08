/*
 * @(#)awt_Panel.h	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

