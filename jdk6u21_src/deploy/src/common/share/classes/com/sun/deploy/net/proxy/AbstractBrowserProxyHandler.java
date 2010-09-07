/*
 * @(#)AbstractBrowserProxyHandler.java	1.54 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.util.StringTokenizer;
import java.net.URL;
import com.sun.deploy.util.Trace;



/**
 * Proxy handler which uses browser to resolve any proxy.
 */
public abstract class AbstractBrowserProxyHandler implements ProxyHandler 
{
    /**
     * Check if the proxy handler supports the proxy type
     *
     * @param proxyType Proxy type
     * @return true if proxy type is supported
     */
    public final boolean isSupported(int proxyType)
    {
	return (proxyType == ProxyType.BROWSER);
    }

    /**
     * Check if the proxy result should be cached
     *
     * @return true if proxy result should be cached
     */
    public final boolean isProxyCacheSupported()
    {
	// Browser proxy setting may be changed by the users
	// in any time, so proxy info should NEVER be cached.
	//
	return false;
    }


    /**
     * Initialize the browser proxy handler.
     *
     * @param info Browser proxy info
     */
    public final void init(BrowserProxyInfo info)
			   throws ProxyConfigException
    {
	Trace.msgNetPrintln("net.proxy.loading.browser");

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
    public final ProxyInfo[] getProxyInfo(URL u)
    {
	// Auto-config Proxy
	String s = findProxyForURL(u.toString());
        return extractAutoProxySetting(s);
    }

    
    /**
     * <p> method to obtain the proxy string when automatic proxy config
     * is used. </p>
     *
     * @param url URL.
     * @returns Proxy string.
     */
    protected abstract String findProxyForURL(String url);

    /* extractAutoProxySetting is a function which takes a proxy-info-string
     * which returned from the JavaScript function FindProxyForURL, and returns
     * the corresponding proxy information.
     *
     * parameters :
     *	s         [in]	a string which contains all the proxy information
     *
     * out:
     *   ProxyInfo [out] ProxyInfo contains the corresponding proxy result.
     *
     * Notes: i) s contains all the proxy information in the form of
     *        "PROXY webcache1-cup:8080;SOCKS webcache2-cup". There are three
     *        possible values inside the string:
     *	        a) "DIRECT" -- no proxy is used.
     *          b) "PROXY"  -- Proxy is used.
     *          c) "SOCKS"  -- SOCKS support is used.
     *        Information for each proxy settings are seperated by ';'. If a
     *	      port number is specified, it is specified by using ':' following
     *        the proxy host.
     *
     */
    private ProxyInfo[] extractAutoProxySetting(String s)
    {
	if (s != null)
	{
	    StringTokenizer st = new StringTokenizer(s, ";", false);
	    ProxyInfo proxyInfoArray[] = new ProxyInfo[st.countTokens()];
	    int index = 0;
	    
	    while (st.hasMoreTokens()) 
	    {  
		String pattern = st.nextToken();     

		int i = pattern.indexOf("PROXY");

		if (i != -1)   {
		    // "PROXY" is specified
		    proxyInfoArray[index++] = new ProxyInfo(pattern.substring(i + 6));
		    continue;
		}

		i = pattern.indexOf("SOCKS");

		if (i != -1) 
		{
		    // "SOCKS" is specified
		    proxyInfoArray[index++] = new ProxyInfo(null, pattern.substring(i + 6));
		    continue;
		}
                // proxy string contains 'DIRECT' or unrecognized text
		proxyInfoArray[index++] = new ProxyInfo(null, -1);
	    }
	    return proxyInfoArray;
	}
	//In order to make the return value safe to use, created a null ProxyInfo object
	return new ProxyInfo[] {new ProxyInfo(null)};
    }
}



