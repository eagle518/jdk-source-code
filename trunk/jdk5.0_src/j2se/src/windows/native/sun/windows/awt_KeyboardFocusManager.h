/*
 * @(#)awt_KeyboardFocusManager.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
