/*
 * @(#)awt_Clipboard.h	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_CLIPBOARD_H
#define AWT_CLIPBOARD_H

#include "awt.h"


/************************************************************************
 * AwtClipboard class
 */

class AwtClipboard {
private:
    static BOOL isGettingOwnership;
    // handle to the next window in the clipboard viewer chain
    static volatile HWND hwndNextViewer;
    static volatile BOOL isClipboardViewerRegistered;
    static volatile BOOL skipInitialWmDrawClipboardMsg;
    static volatile jmethodID handleContentsChangedMID;

public:
    static jmethodID lostSelectionOwnershipMID;
    static jobject theCurrentClipboard;

    INLINE static void GetOwnership() {
	AwtClipboard::isGettingOwnership = TRUE;
	VERIFY(EmptyClipboard());
	AwtClipboard::isGettingOwnership = FALSE;
    }

    INLINE static BOOL IsGettingOwnership() {
	return isGettingOwnership;
    }

    static void LostOwnership(JNIEnv *env);
    static void WmChangeCbChain(WPARAM wparam, LPARAM lparam);
    static void WmDrawClipboard(JNIEnv *env, WPARAM wparam, LPARAM lparam);
    static void RegisterClipboardViewer(JNIEnv *env, jobject jclipboard);
    static void UnregisterClipboardViewer(JNIEnv *env);
};

#endif /* AWT_CLIPBOARD_H */
