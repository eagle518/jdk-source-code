/*
 * @(#)awt_Window.h	1.14 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni_util.h>

/* fieldIDs for Window fields that may be accessed from C */
struct WindowIDs {
    jfieldID warningString;
    jmethodID resetGCMID;
    jmethodID focusableWindowMID;
    jfieldID focusableWindowState;
    jfieldID locationByPlatform;
};

/* fieldIDs for MWindowPeer fields that may be accessed from C */
struct MWindowPeerIDs {
    jfieldID insets;
    jfieldID iconWidth;
    jfieldID iconHeight;
    jfieldID winAttr;
    jmethodID handleWindowFocusIn;
    jmethodID handleWindowFocusOut;
    jmethodID handleIconify;
    jmethodID handleDeiconify;
    jmethodID handleStateChange;
    jmethodID draggedToScreenMID;
};

