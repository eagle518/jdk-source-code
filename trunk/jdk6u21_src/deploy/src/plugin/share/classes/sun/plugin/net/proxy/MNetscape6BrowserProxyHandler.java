/*
 * @(#)MNetscape6BrowserProxyHandler.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

import sun.plugin.navig.motif.Worker;

/**
 * Proxy handler for Netscape 6.
 */
public final class MNetscape6BrowserProxyHandler extends com.sun.deploy.net.proxy.AbstractBrowserProxyHandler
{
    /**
     * <p> method to obtain the proxy string when automatic proxy config
     * is used. </p>
     *
     * @param url URL.
     * @returns Proxy string.
     */
    protected String findProxyForURL(String url) {
        return Worker.getProxySettings(url);
    }
}
