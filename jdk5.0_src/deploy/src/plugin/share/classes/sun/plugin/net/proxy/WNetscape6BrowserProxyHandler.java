/*
 * @(#)WNetscape6BrowserProxyHandler.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

/**
 * Proxy handler for Netscape 6.
 */
public final class WNetscape6BrowserProxyHandler extends com.sun.deploy.net.proxy.AbstractBrowserProxyHandler
{
    /**
     * <p> method to obtain the proxy string when automatic proxy config
     * is used. </p>
     *
     * @param url URL.
     * @returns Proxy string.
     */
    protected native String findProxyForURL(String url);
}
