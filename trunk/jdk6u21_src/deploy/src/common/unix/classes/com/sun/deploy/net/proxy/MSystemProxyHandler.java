/*
 * @(#)AbstractBrowserProxyHandler.java	1.52 03/12/19
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.util.StringTokenizer;
import java.net.URL;
import com.sun.deploy.util.Trace;
import java.net.*;

/**
 * Proxy handler which uses System to resolve any proxy. suit for platform unix
 * and mozilla
 */
public class MSystemProxyHandler implements ProxyHandler {

	
	protected static boolean hasSystemProxies = false;
	static {
		hasSystemProxies = init();
    }

	/**
	 * Check if the proxy handler supports the proxy type
	 * 
	 * @param proxyType
	 *            Proxy type
	 * @return true if proxy type is supported
	 */
	public final boolean isSupported(int proxyType) {
		return (proxyType == ProxyType.SYSTEM);
	}

	/**
	 * Check if the proxy result should be cached
	 * 
	 * @return true if proxy result should be cached
	 */
	public final boolean isProxyCacheSupported() {
		// System proxy setting may be changed by the users
		// in any time, so proxy info should NEVER be cached.
		//
		return false;
	}
	
	/**
     * Initialize the System proxy handler.
     *
     * @param info System proxy info
     */
    public final void init(BrowserProxyInfo info)
			   throws ProxyConfigException
    {
	Trace.msgNetPrintln("net.proxy.loading.system");

	// Check if proxy type is supported
	if (isSupported(info.getType()) == false)
	    throw new ProxyConfigException("Unable to support proxy type: " + info.getType());

	Trace.msgNetPrintln("net.proxy.loading.done");
    }
    
    /** 
     * Return proxy info for a given URL
     *
     * @param u URL
     * @return proxy info for a given URL
     */
    public final ProxyInfo[] getProxyInfo(URL u) throws ProxyUnavailableException {
    	ProxyInfo proxyInfoArray[] = new ProxyInfo[1];
    	
    	String proto  = u.getProtocol();
    	String host = u.getHost();
    	
    	if(hasSystemProxies) {
    		//call native method to get system proxy
    		String p = getSystemProxy(proto,host);
    		if(p == null) return new ProxyInfo[] {new ProxyInfo(null)};
        	
        	//convert proxy to proxyInfo and return proxyInfoArray
        	//ProxyInfo format is like "webcache.XX.XX:8080"
        	proxyInfoArray[0] = new ProxyInfo(p);
        } else {
        	return new ProxyInfo[] {new ProxyInfo(null)};
        }
    	
    	return proxyInfoArray;
    }
    
    protected native static boolean init();
    protected native String getSystemProxy(String protocol, String host);
}
