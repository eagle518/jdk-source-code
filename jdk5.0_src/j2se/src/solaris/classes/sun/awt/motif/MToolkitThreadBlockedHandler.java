/*
 * @(#)MToolkitThreadBlockedHandler.java	1.5 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.awt.datatransfer.ToolkitThreadBlockedHandler;

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
        AWTLockAccess.awtLock();
    }
    public void unlock() {
        AWTLockAccess.awtUnlock();
    }
    public native void enter();
    public native void exit();
}
