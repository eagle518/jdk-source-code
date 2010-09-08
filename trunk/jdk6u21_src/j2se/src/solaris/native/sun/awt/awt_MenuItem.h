/*
 * @(#)awt_MenuItem.h	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni_util.h>

/* fieldIDs for MenuItem fields that may be accessed from C */
struct MenuItemIDs {
    jfieldID label;
    jfieldID enabled;
    jfieldID shortcut;
};

/* fieldIDs for MMenuItemPeer fields that may be accessed from C */
struct MMenuItemPeerIDs {
    jfieldID target;
    jfieldID pData;
    jfieldID isCheckbox;
    jfieldID jniGlobalRef;
};
