/*
 * @(#)DirectProxyHandler.java	1.25 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.net.proxy;

import java.net.URL;
import com.sun.deploy.util.Trace;


/**
 * Proxy handler for Direct connection.
 */
final class DirectProxyHandler implements ProxyHandler 
{
    /**
     * Check if the proxy handler supports the proxy type
     *
     * @param proxyType Proxy type
     * @return true if proxy type is supported
     */
    public boolean isSupported(int proxyType)
    {
	return (proxyType == ProxyType.NONE);
    }

    /**
     * Check if the proxy result should be cached
     *
     * @return true if proxy result should be cached
     */
    public boolean isProxyCacheSupported()
    {
	return true;	    
    }

    /**
     * Initialize the direct proxy handler.
     *
     * @param info Browser proxy info
     */
    public void init(BrowserProxyInfo info)
		throws ProxyConfigException
    {
	Trace.msgNetPrintln("net.proxy.loading.direct");

	// Check if proxy type is supported
	if (isSupported(info.getType()) == false)
	    throw new ProxyConfigException("Unable to support proxy type: " + info.getType());

	Trace.msgNetPrintln("net.proxy.loading.done");
    }


    /**
     * Returns proxy info for a given URL
     *
     * @param u URL
     * @return proxy info for a given URL
     */
    public ProxyInfo[] getProxyInfo(URL u)
    {
	return new ProxyInfo[] {new ProxyInfo(null)};
    }
}
