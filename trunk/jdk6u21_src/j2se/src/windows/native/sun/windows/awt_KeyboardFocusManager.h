/*
 * @(#)awt_KeyboardFocusManager.h	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_KEYBOARDFOCUSMANAGER_H
#define AWT_KEYBOARDFOCUSMANAGER_H

#include <jni.h>

class AwtKeyboardFocusManager {
public:

    static jclass keyboardFocusManagerCls;
    static jmethodID shouldNativelyFocusHeavyweightMID;
    static jmethodID heavyweightButtonDownMID;
    static jmethodID markClearGlobalFocusOwnerMID;
    static jmethodID removeLastFocusRequestMID;
    static jfieldID isProxyActive;
    static jmethodID processSynchronousTransfer;
};

#endif // AWT_KEYBOARDFOCUSMANAGER_H
