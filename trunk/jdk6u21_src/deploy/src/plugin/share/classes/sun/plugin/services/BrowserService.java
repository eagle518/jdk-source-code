/*
 * @(#)BrowserService.java	1.31 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
     * NOTE that this returns Object instead of sun.plugin.viewer.context.PluginAppletContext
     * to break compile-time dependencies
     */
    public Object getAppletContext();

    /**
     * Return beans context
     * NOTE that this returns Object instead of sun.plugin.viewer.context.PluginBeansContext
     * to break compile-time dependencies
     */
    public Object getBeansContext();

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



