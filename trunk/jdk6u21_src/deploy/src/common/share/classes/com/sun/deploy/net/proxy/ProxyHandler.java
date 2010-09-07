/*
 * @(#)ProxyHandler.java	1.25 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.net.URL;


public interface ProxyHandler {

    /**
     * Check if the proxy handler supports the proxy type
     *
     * @param proxyType Proxy type
     * @return true if proxy type is supported
     */
    boolean isSupported(int proxyType);

    /**
     * Check if the proxy result should be cached
     *
     * @return true if proxy result should be cached
     */
    boolean isProxyCacheSupported();

    /**
     * Initialize the proxy handler.
     *
     * @param info Browser proxy info
     */
    void init(BrowserProxyInfo info) throws ProxyConfigException;

    /** 
     * Return proxy info for a given URL
     *
     * @param u URL
     * @return proxy info for a given URL
     */
    ProxyInfo[] getProxyInfo(URL u) throws ProxyUnavailableException;
}



