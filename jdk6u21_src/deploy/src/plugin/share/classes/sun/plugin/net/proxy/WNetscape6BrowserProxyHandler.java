/*
 * @(#)WNetscape6BrowserProxyHandler.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
