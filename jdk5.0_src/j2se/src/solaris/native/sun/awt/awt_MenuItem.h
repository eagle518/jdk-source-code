/*
 * @(#)awt_MenuItem.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
