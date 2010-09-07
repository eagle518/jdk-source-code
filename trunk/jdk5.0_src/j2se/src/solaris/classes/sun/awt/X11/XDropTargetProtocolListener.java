/*
 * @(#)XDropTargetProtocolListener.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

interface XDropTargetProtocolListener {
    void handleDropTargetNotification(XWindow xwindow, int x, int y, 
                                      int dropAction, int actions, 
                                      long[] formats, long nativeCtxt, 
                                      int eventID);
}
