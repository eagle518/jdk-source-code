/*
 * @(#)MToolkitThreadBlockedHandler.java	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.awt.datatransfer.ToolkitThreadBlockedHandler;
import sun.awt.SunToolkit;

final class MToolkitThreadBlockedHandler implements 
                                 ToolkitThreadBlockedHandler {
    private static ToolkitThreadBlockedHandler priveleged_lock = null;
    static {
        priveleged_lock = new MToolkitThreadBlockedHandler();    
    }
    private MToolkitThreadBlockedHandler() {}
    static ToolkitThreadBlockedHandler getToolkitThreadBlockedHandler() {
        return priveleged_lock;
    }
    public void lock() {
        SunToolkit.awtLock();
    }
    public void unlock() {
        SunToolkit.awtUnlock();
    }
    public native void enter();
    public native void exit();
}
