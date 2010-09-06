/*
 * @(#)IExplorerAppletStatusListener.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer;

/**
 * IExplorerAppletStatusListener is a concrete class for listening applet
 * status changes.
 */
public class IExplorerAppletStatusListener implements sun.plugin.AppletStatusListener
{    
    private int handle = 0;

    IExplorerAppletStatusListener(int handle)
    {
	this.handle = handle;
    }
    
    /**
     * Notify applet status change
     */
    public void statusChanged(int status)
    {
	notifyStatusChange(handle, status);
    }

    // Native code to notify the changes
    private native void notifyStatusChange(int handle, int status);
}   


