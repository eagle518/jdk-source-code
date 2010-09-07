/*
 * @(#)awt_KeyboardFocusManager.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"

struct KeyboardFocusManagerIDs {
    jclass keyboardFocusManagerCls;
    jmethodID shouldNativelyFocusHeavyweightMID;
    jmethodID heavyweightButtonDownMID;
    jmethodID heavyweightButtonDownZMID;
    jmethodID markClearGlobalFocusOwnerMID;
    jmethodID processSynchronousTransferMID;
    jfieldID isProxyActive;
};
