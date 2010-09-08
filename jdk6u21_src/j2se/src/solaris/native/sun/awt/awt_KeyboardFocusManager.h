/*
 * @(#)awt_KeyboardFocusManager.h	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
