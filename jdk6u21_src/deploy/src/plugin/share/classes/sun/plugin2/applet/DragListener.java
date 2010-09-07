/*
 * @(#)DragListener.java	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

/** An interface providing interaction with the drag and drop gesture
    placing the applet on the desktop. */

public interface DragListener {
    public void appletDroppedOntoDesktop(Plugin2Manager manager);
    public void appletExternalWindowClosed(Plugin2Manager manager);
}
