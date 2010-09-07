/*
 * @(#)PluginAppletContext.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.context;

import java.applet.AppletContext;
import java.applet.Applet;
import sun.applet.AppletPanel;
import sun.plugin.javascript.JSContext;


/**
 * This class corresponds to an extended interface of the 
 * AppletContext.
 */
public interface PluginAppletContext extends AppletContext, JSContext
{
    /**
     * Add applet panel in current Applet context.
     *
     * @param panel Applet Panel object
     */
    public void addAppletPanelInContext(AppletPanel panel);

    /**
     * Remove applet panelfrom current Applet context.
     *
     * @param panel Applet Panel object
     */
    public void removeAppletPanelFromContext(AppletPanel panel);

    /**
     * Set the underlying handle of the Applet context
     * 
     * @param handle Handle
     */
    public void setAppletContextHandle(int handle);
}


