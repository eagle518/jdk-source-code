/*
 * @(#)BrowserService.java	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import com.sun.deploy.services.Service;


/** 
 * BrowserService is an interface that encapsulates the browser service.
 */
public interface BrowserService extends Service
{
    /**
     * Return applet context
     */
    public sun.plugin.viewer.context.PluginAppletContext getAppletContext();

    /**
     * Return beans context
     */
    public sun.plugin.viewer.context.PluginBeansContext getBeansContext();

    /**
     * Check if browser is IE.
     */
    public boolean isIExplorer();

    /**
     * Check if browser is NS.
     */
    public boolean isNetscape();

    /**
     * Return browser version.
     */
    public float getBrowserVersion();

    /**
     * Check if console should be iconified on close.
     */
    public boolean isConsoleIconifiedOnClose();

    /**
     * Install browser event listener
     * @since 1.4.1
     */
    public boolean installBrowserEventListener();

	/**
	 * Browser authenticator
	 * @since 1.4.2
	 */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator();

    /**
     * Browser element mapping
     * @since 1.4.2
     */
    public String mapBrowserElement(String rawName);
}



