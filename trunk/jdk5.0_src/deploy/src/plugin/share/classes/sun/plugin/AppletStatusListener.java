/*
 * @(#)AppletStatusListener.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/**
 * AppletStatusListener is an interface for listening applet
 * status changes.
 */
public interface AppletStatusListener
{    
    /**
     * Notify applet status change
     */
    public void statusChanged(int status);
}   

