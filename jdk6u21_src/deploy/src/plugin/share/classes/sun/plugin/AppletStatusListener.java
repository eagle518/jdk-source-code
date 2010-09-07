/*
 * @(#)AppletStatusListener.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

