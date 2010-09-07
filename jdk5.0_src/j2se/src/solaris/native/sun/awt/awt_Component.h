/*
 * @(#)awt_Component.h	1.26 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni_util.h"

/* fieldIDs for Component fields that may be accessed from C */
struct ComponentIDs {
    jfieldID x;
    jfieldID y;
    jfieldID width;
    jfieldID height;
    jfieldID peer;
    jfieldID background;
    jfieldID foreground;
    jfieldID isPacked;
    jfieldID graphicsConfig;
    jfieldID privateKey;
    jfieldID name;
    jfieldID isProxyActive;
    jfieldID appContext;
    jmethodID getParent;
    jmethodID getLocationOnScreen;
    jmethodID resetGCMID;
};

/* field and method IDs for Container */
struct ContainerIDs {
    jfieldID layoutMgr;
    jmethodID getComponents;
    jmethodID findComponentAt;
};

/* fieldIDs for MComponentPeer fields that may be accessed from C */
struct MComponentPeerIDs {
    jfieldID pData;
    jfieldID target;
    jfieldID jniGlobalRef;
    jfieldID graphicsConfig;
    jfieldID drawState;
    jmethodID isFocusableMID;
};

#ifndef HEADLESS
extern void processTree(Widget from, Widget to, Boolean action);
#endif // HEADLESS

/* fieldIDs for Canvas fields that may be accessed from C */
struct CanvasIDs {
    jmethodID setGCFromPeerMID;
};

