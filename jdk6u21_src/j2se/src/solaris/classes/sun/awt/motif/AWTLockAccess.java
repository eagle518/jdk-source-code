/*
 * @(#)AWTLockAccess.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

final class AWTLockAccess {
    static native void awtLock();
    static native void awtUnlock();
    static void awtWait() { awtWait(0); }
    static native void awtWait(long timeout);
    static native void awtNotifyAll();
}
