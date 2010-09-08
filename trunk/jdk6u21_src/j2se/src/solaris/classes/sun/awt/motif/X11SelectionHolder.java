/*
 * @(#)X11SelectionHolder.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

interface X11SelectionHolder {

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void lostSelectionOwnership();
}
