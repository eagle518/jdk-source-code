/*
 * @(#)ToolkitThreadBlockedHandler.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.datatransfer;

public interface ToolkitThreadBlockedHandler {
    public void lock();
    public void unlock();
    public void enter();
    public void exit();
}
